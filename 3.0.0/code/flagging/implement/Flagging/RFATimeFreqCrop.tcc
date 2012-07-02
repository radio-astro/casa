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
#define Nmt (cubepos[2]/NumB)

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
	BASELN_TOL = parm.asDouble("bs_cutoff");
	T_TOL = parm.asDouble("time_amp_cutoff");
	F_TOL = parm.asDouble("freq_amp_cutoff");
	FlagLevel = parm.asInt("flag_level");
	CorrChoice = parm.asInt("auto_cross");
	StartChan = parm.asInt("start_chan");
	EndChan = parm.asInt("end_chan");
	NumTime = parm.asInt("num_time");
	ShowPlots = parm.asBool("showplots");
	FreqLineFit = parm.asBool("freqlinefit");
	cout << "Flagging on " << parm.asArrayString(RF_EXPR) << " for column : " << parm.asString(RF_COLUMN) << endl;
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
	rec.define("bs_cutoff",0);
	rec.define("time_amp_cutoff",3);
	rec.define("freq_amp_cutoff",3);
	rec.define("flag_level",1);
	rec.define("auto_cross",0);
	rec.define("start_chan",0);
	rec.define("end_chan",0);
	rec.define("num_time",50);
	rec.define("column","DATA");
	rec.define("expr","ABS I");
	rec.define("fignore",False);
	rec.define("showplots",False);
	rec.define("freqlinefit",False);
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
 corrmask = RFDataMapper::corrMask(chunk.visIter());
 
	cout << "Working with data chunk : " << chunk.msName() << endl;
	cout << "TimeSteps = " << num(TIME) << ", Baselines = " << num(IFR) << ", Chans = " << num(CHAN) << ", Polns = " << num(POLZN) << ", Ants = " << num(ANT) << endl;
	cout << "Parameters : " << " Antenna_tol=" << ANT_TOL << ", Baseline_tol=" << BASELN_TOL << ", Time_tol=" << T_TOL << "sigma, Freq_tol=" << F_TOL << "sigma, FlagLevel=" << FlagLevel << ", Flag_corr=" << String(CorrChoice?"Cross":"Auto") << endl;

	nPol = 1;
	
	//flg = RFADiffMapBase::newChunk(i);

	// Initialize NumT - number of timestamps to work with in one go

	if(NumTime==0) NumTime=50;
	if(num(TIME) >= NumTime) NumT = NumTime;
	else NumT = num(TIME);
	
	///flag_iter = flag.newCustomIter();

       NumAnt = num(ANT);
       NumB = ((NumAnt)*((NumAnt)-1)/2 + (NumAnt));

       RFAFlagCubeBase::newChunk(i-=1);
       
	return True;
}


/* Called at the beginning of each PASS */
void RFATimeFreqCrop :: startData () 
{
	RFAFlagCubeBase::startData();
	iterTimecnt=0; // running count of visbuffers gone by.
	timecnt=0;
  
	gnu = popen("gnuplot","w");
	fprintf(gnu,"set nokey\n");
	
	(chunk.visIter()).setRowBlocking(0); 

	///flag_iter.reset();// reset the local flag_iter to beginning of pass/chunk
	
//	if(CorrChoice==0 && vb.nRow()) ---> Check what to do if there are no selfs and the user asks to operate on selfs........

	/* If channel range is invalid - reset to the complete range */
	if(StartChan<0 || StartChan >= num(CHAN)-10) StartChan=0;
	if(EndChan <= 10 || EndChan >=num(CHAN)) EndChan=num(CHAN)-1;
	if(EndChan <= StartChan) {StartChan=0;EndChan=num(CHAN)-1;}
	// print a warning

	
	/* Cube to hold visibility amplitudes : POLZN x CHAN x (IFR*TIME) */
	visc.resize((uInt)(nPol),(uInt)(EndChan-StartChan+1),(uInt)(NumB*NumT));
	visc=0;
	cubepos = visc.shape();
	//cout << " VisCubePos = " << cubepos << endl;

	
	/* Cube to hold visibility flags : POLZN x CHAN x (IFR*TIME) */
	flagc.resize(cubepos);
	flagc=False;
	cubepos = flagc.shape();
	//cout << " FlagCubePos = " << cubepos << endl;
	
	/* Cube to hold flags for the entire Chunk (channel subset, but all times) */
	chunkflags.resize((uInt)(nPol),(uInt)(EndChan-StartChan+1),(uInt)(NumB*num(TIME)));

	/* Cube to hold MEAN bandpasses : POLZN x IFR x CHAN */
	if(CorrChoice == 0)
		meanBP.resize((uInt)cubepos[0],(uInt)(NumAnt),(uInt)cubepos[1]);
	else
		meanBP.resize((uInt)cubepos[0],(uInt)NumB,(uInt)cubepos[1]);
	
	meanBP=0;
	matpos = meanBP.shape();
	//cout << " MeanBPPos = " << matpos << endl;

	/* Cube to hold CLEAN bandpasses : POLZN x IFR x CHAN */
	if(CorrChoice==0)
		cleanBP.resize((uInt)cubepos[0],(uInt)(NumAnt),(uInt)cubepos[1]);
	else
		cleanBP.resize((uInt)cubepos[0],(uInt)NumB,(uInt)cubepos[1]);
	
	cleanBP=0;
	matpos = cleanBP.shape();
	//cout << " CleanBPPos = " << matpos << endl;

	/* Matrix to hold Row Flags : POLZX x (IFR*TIME) */
	RowFlags.resize((uInt)cubepos[0],(uInt)(NumB*NumT));
	RowFlags=False;
	//cout << "RowFlags = " << RowFlags.shape() << endl;
	
	
	/* Temporary workspace vectors */	
	tempBP.resize(cubepos[1]);tempTS.resize(NumT);
	flagBP.resize(cubepos[1]);flagTS.resize(NumT);
	fitBP.resize(cubepos[1]);fitTS.resize(NumT);

	tempBP=0;tempTS=0;flagBP=False;flagTS=False;fitBP=0;fitTS=0;


//	cout << "Start Data " <<endl;
}



/* Called once for every TIMESTAMP - for each VisBuf */
RFA::IterMode RFATimeFreqCrop :: iterTime (uInt itime) 
{
    Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
    uInt a1,a2,antcnt;
    Bool flg=False;
    
    RFDataMapper::setVisBuffer(vb);
    
    flag.advance(itime,True);
    
    //vc = vb.visCube(); // extract a viscube - one timestamp - one VisBuf
    vv = &vb.visCube(); // extract a viscube - one timestamp - one VisBuf
    vi.flag(ff); // extract the corresponding flags
    
    if(ant1.shape() != (vb.antenna1()).shape())
	ant1.resize((vb.antenna1()).shape());
    ant1 = vb.antenna1();
    
    if(ant2.shape() != (vb.antenna2()).shape())
	ant2.resize((vb.antenna2()).shape());
    ant2 = vb.antenna2();
    const Vector<Int> &ifrs( chunk.ifrNums() );
    
    //uInt npols = vv->nrow();
    uInt npols = 1;
    uInt nbs = ant1.nelements();
    
    /*
      cout << " Nrows in viscube = " << " " << vv->nrow() << endl;
      cout << " Ncolumns in viscube = " << " " << vv->ncolumn() << endl;
      cout << " Nplanes in viscube = " << " " << vv->nplane()<< endl;
      
      cout << " Nrows in flagcube = " << ff.nrow() << endl;
      cout << " Ncolumns in flagcube = " << ff.ncolumn() << endl;
      cout << " Nplanes in flagcube = " << ff.nplane() << endl;
      
      cout << " itime : " << itime << "  Nelements in ant1 = " << ant1.nelements() << endl;
      cout << " itime : " << itime << "  Nelements in ant2 = " << ant2.nelements() << endl;
    */
    
	
    /* BEGIN TIME-FLAGGING ALGORITHM HERE */

	/* Read in the data AND any existing flags into the flagCube - accumulate */
	for(uInt pl=0;pl<npols;pl++)
	{
	  for(uInt ch=StartChan;ch<=EndChan;ch++)
	  {
	    for(uInt bs=0;bs<nbs;bs++)
	    {
	      //visc(pl,ch-StartChan,(timecnt*NumB)+baselinecnt) = static_cast<Float>(abs((*vv)(pl,ch,bs)) );
	      //flagc(pl,ch-StartChan,(timecnt*NumB)+baselinecnt) = ff(pl,ch,bs);
	      //chunkflags(pl,ch-StartChan,(itime*NumB)+baselinecnt) = ff(pl,ch,bs);
		    
	      ///visc(pl,ch-StartChan,(timecnt*NumB)+baselinecnt) = mapValue(ch,bs);
	      ///flagc(pl,ch-StartChan,(timecnt*NumB)+baselinecnt) = flag.getFlag(ch,ifrs(bs));//flag.anyFlagged(ch,ifrs(bs));
	      ///chunkflags(pl,ch-StartChan,(itime*NumB)+baselinecnt) = flag.getFlag(ch,ifrs(bs));//flag.anyFlagged(ch,ifrs(bs));
	      
	      visc(pl,ch-StartChan,(timecnt*NumB)+baselinecnt) = mapValue(ch,bs);
	      flagc(pl,ch-StartChan,(timecnt*NumB)+baselinecnt) = flag.anyFlagged(ch,ifrs(bs));
	      chunkflags(pl,ch-StartChan,(itime*NumB)+baselinecnt) = flag.anyFlagged(ch,ifrs(bs));
	    }
	  }
	}
	timecnt++;
	iterTimecnt++; // running count of visbuffers going by.

	/* After accumulating NumT timestamps, start processing this block */
	/////if(iterTimecnt > 0 && (timecnt==NumT || iterTimecnt == (vi.nRowChunk()/NumB)))
	if(iterTimecnt > 0 && (timecnt==NumT || itime==(num(TIME)-1) ))
	    {
		/* Update RowFlags - Flags to indicate which baselines are missing in
		 * each integration timestamp */
		
		RowFlags=False;
		
		for(int pl=0;pl<matpos[0];pl++)
		    {
			for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
			    {
				for(uInt bs=0;bs<NumB;bs++)
				    {
					temp=0;
					flg=True;
					for(int ch=0;ch<matpos[2];ch++)
					    {
						temp += visc(pl,ch,tm*NumB+bs);
						flg &= flagc(pl,ch,tm*NumB+bs);
					    }
					RowFlags(pl,tm*NumB+bs)=flg;
					if(temp==0) 
					    {
						RowFlags(pl,tm*NumB+bs)=True;
						for(int ch=0;ch<matpos[2];ch++)
						    flagc(pl,ch,tm*NumB+bs)=True;
					    }
				    }// for bs
			    }// for tm
		    }// for pl
		
		
		/* For each Channel - fit lines to 1-D data in time - flag according 
		 * to them and build up the mean bandpass */
		
		//flagc = (Bool)False;
		meanBP = 0;
		cleanBP = 0;
		flagcnt = 0;
		antcnt = 0;
		
		cout << endl << " Flag across " << timecnt << " timesteps and create a time-averaged bandpass " << endl;
		
		Float rmean=0;
		for(int pl=0;pl<matpos[0];pl++)
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
			for(int ch=0;ch<matpos[2];ch++)
			{
				tempTS=0;flagTS=False;
				for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
				{
					tempTS[tm] = visc(pl,ch,((tm*NumB)+bs));
					flagTS[tm] = flagc(pl,ch,((tm*NumB)+bs));
				}//for tm
			
				rmean += UMean(tempTS,flagTS);
				
				temp=0;
				for(int loop=0;loop<5;loop++)
				{
				// HERE - give choice of PolyFit in time...
				LineFit(tempTS,flagTS,fitTS,0,tempTS.nelements()-1);	
				sd = UStd(tempTS,flagTS,fitTS);
			 			
				for(uInt i=0;i<cubepos[2]/NumB;i++)
					if(flagTS[i]==False && fabs(tempTS[i]-fitTS[i]) > T_TOL*sd)
					{
						flagTS[i]=True ;flagcnt++;
					}
				if(fabs(temp-sd) < 0.1)break;
				temp=sd;
				}
				
				meanBP(pl,bs,ch) = UMean(tempTS,flagTS) ;
					
				/* write flags to local flag cube */
				for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
					flagc(pl,ch,((tm*NumB)+bs))=flagTS[tm];
			
			}//for ch
			
			antcnt++; 
			
			/* Check for completely flagged ants/bs */
			if((CorrChoice==0 && a1 == a2)||(CorrChoice!=0 && a1 != a2)) 
			{
				if(CorrChoice==0)tol=ANT_TOL;
				else tol=BASELN_TOL;
				if(rmean/matpos[2] < tol)
				{
					for(int ch=0;ch<matpos[2];ch++)
				        for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
						flagc(pl,ch,((tm*NumB)+bs))=True;
					if(CorrChoice==0) 
						cout << "Antenna Flagged : " << a1 << endl;
					else
						cout << "Baseline Flagged : " << a1 << ":" << a2 << endl;
				}
				
			}		
		}//for bs
	}//for pl
	

/* Fit a smooth bandpass to the mean bandpass and store it 
 *  one for each baseline */

//cout << " Clean up the bandpasses ... " << endl;

for(int pl=0;pl<matpos[0];pl++)
{
	for(int ant=0;ant<matpos[1];ant++)
	{
	
	/* Fit a smooth bandpass */
	flg=True;
	for(int ch=0;ch<matpos[2];ch++)	
	{
		tempBP[ch] = meanBP(pl,ant,ch);
		if(tempBP[ch] > 0) flg=False;
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
		UPlot(tempBP,fitBP,0,tempBP.nelements()-1);  // Plot the band
#endif
	}
	
	for(int ch=0;ch<matpos[2];ch++)
	{
		if(flg==False) cleanBP(pl,ant,ch)= fitBP[ch];
		else cleanBP(pl,ant,ch)=0;
	}
	
	
	}//for bs
}//for pl
/* END FLAGGING IN TIME */
		
	
#if 1

/* FLAGGING IN FREQUENCY */

cout << " Fit the bandpass shape and flag across " << matpos[2] << " frequency channels" << endl;

for(int pl=0;pl<matpos[0];pl++)
{
	antcnt=0;
	for(uInt bs=0;bs<NumB;bs++)
	{
		Ants(bs,&a1,&a2);
		if(CorrChoice==0)
		{if(a1 != a2) continue;} // choose auto correlations
		else
		{if(a1==a2)continue;} // choose cross correlations
		
		for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
		{
			tempBP=0,flagBP=0;
			for(int ch=0;ch<matpos[2];ch++)
			{
				flagBP[ch] = flagc(pl,ch,((tm*NumB)+bs));
				if(flagBP[ch]==False)
				tempBP[ch] = visc(pl,ch,((tm*NumB)+bs))/cleanBP(pl,bs,ch);
			}//for ch
		
			temp=0;
			for(Int loop=0;loop<5;loop++)
			{
			mn=1;
			sd = UStd(tempBP,flagBP,mn);
			
			for(Int i=0;i<matpos[2];i++)
				if(flagBP[i]==False && fabs(tempBP[i]-mn) > F_TOL*sd)
				{
					flagBP[i]=True ;flagcnt++;
				}
			if(fabs(temp-sd) < 0.1)break;
			temp=sd;
			}

			for(Int ch=0;ch<matpos[2];ch++)
				flagc(pl,ch,((tm*NumB)+bs))=flagBP[ch];
			
			
		}//for tm
		if((CorrChoice==0 && a1 == a2) || (CorrChoice!=0 && a1 != a2)) 
		 antcnt++;
	}//for bs
}//for pl	

/* END FLAGGING IN FREQUENCY */


/* APPLY FLAG HEURISTICS ON THE FLAGS FOR ALL AUTOCORRELATIONS */

cout << " Apply Flag Heuristics for flaglevel " << FlagLevel << endl;


if(FlagLevel > 0)
{
for(int pl=0;pl<matpos[0];pl++)
{
	for(uInt bs=0;bs<NumB;bs++)
	{
		Ants(bs,&a1,&a2);
		if(CorrChoice==0)
		{if(a1 != a2) continue;} // choose autocorrelations
		else
		{if(a1==a2) continue;} // choose cross correlations
		
		for(int ch=0;ch<matpos[2];ch++)
		{ 
			uInt fsum=0;
			for(uInt tm=0;tm<Nmt;tm++)
			{       
				// flagging one timestamp before and after
				if(tm>0)
				  if(flagc(pl,ch,((tm*NumB+bs)))==True)
				    flagc(pl,ch,(((tm-1)*NumB+bs)))=True;

				if((Nmt-tm)<Nmt-1)
				  if(flagc(pl,ch,(((Nmt-tm)*NumB+bs)))==True)
				    flagc(pl,ch,(((Nmt-tm+1)*NumB+bs)))=True;

				// flagging one channel before and after
				if(FlagLevel>1)
				{
				if(ch>0)
				  if(flagc(pl,ch,((tm*NumB+bs)))==True)
				     flagc(pl,ch-1,(((tm)*NumB+bs)))=True;

				if((matpos[2]-ch)<matpos[2]-1)
				  if(flagc(pl,(matpos[2]-ch),(tm*NumB+bs))==True)
				    flagc(pl,(matpos[2]-ch+1),(tm*NumB+bs))=True;
				}

				// if more than 60% of a channel flagged - flag whole channel
				if(FlagLevel>1) // flag level 2
				fsum += (flagc(pl,ch,((tm*NumB+bs)))==True)?0:1 ; 

			}//for tm
			
			if(fsum < 0.4*cubepos[2]/NumB && FlagLevel > 1) // flag level 2 
				for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
					flagc(pl,ch,((tm*NumB+bs)))=True;
		}//for ch
	}//for bs
}//for pl

}//if flag_level


/* GRAY SCALE DISPLAYS ON ds9 */

//#ifdef DOPLOT

if(ShowPlots)
{

Float **dispdat=NULL,**flagdat=NULL;
dispdat = (Float **) malloc(sizeof(Float *) * cubepos[2]/NumB + sizeof(Float)*matpos[2]*cubepos[2]/NumB);
for(uInt i=0;i<cubepos[2]/NumB;i++)
dispdat[i] = (Float *) (dispdat + cubepos[2]/NumB) + i * matpos[2];

flagdat = (Float **) malloc(sizeof(Float *) * cubepos[2]/NumB + sizeof(Float)*matpos[2]*cubepos[2]/NumB);
for(uInt i=0;i<cubepos[2]/NumB;i++)
flagdat[i] = (Float *) (flagdat + cubepos[2]/NumB) + i * matpos[2];

Float runningsum=0, runningflag=0;
for(int pl=0;pl<matpos[0];pl++)
{
	for(uInt bs=0;bs<NumB;bs++)
	{
		runningsum=0;
		runningflag=0;
		Ants(bs,&a1,&a2);
		if(CorrChoice==0)
		{if(a1 != a2) continue;} // choose autocorrelations
		else
		{if(a1==a2) continue;} // choose cross correlations

		for(int ch=0;ch<matpos[2];ch++)	
		{
			tempBP[ch] = meanBP(pl,bs,ch);
			fitBP[ch] = cleanBP(pl,bs,ch);
		}
		
		for(int ch=0;ch<matpos[2];ch++)
		{ 
			for(uInt tm=0;tm<Nmt;tm++)
			{       
				dispdat[tm][ch] = visc(pl,ch,(((tm*NumB)+bs)));
				flagdat[tm][ch] = dispdat[tm][ch]*(!flagc(pl,ch,(tm*NumB)+bs));
				runningsum += dispdat[tm][ch];
				runningflag += Float(flagc(pl,ch,(tm*NumB)+bs));
			}//for tm
		}//for ch

		//Plot on ds9 !!
		
		cout << "Antenna1 : " << a1 << "  Antenna2 : " << a2 << "  Polarization : " << pl << endl;
		cout << "Vis sum : " << runningsum << " Flag % : " << 100 * runningflag/(matpos[2]*Nmt) << endl;
		if(runningsum)
		{
		Display_ds9(matpos[2],cubepos[2]/NumB,dispdat,1);
		Display_ds9(matpos[2],cubepos[2]/NumB,flagdat,2);
		UPlot(tempBP,fitBP,0,tempBP.nelements()-1);  // Plot the band
		}
	}//for bs
}//for pl

}// end of if ShowPlots

//#endif

/* FLAG BASELINES FROM THE SELF FLAGS */

if(CorrChoice ==0)
{	
cout << " Flagging Cross correlations from self correlation flags " << endl;
for(int pl=0;pl<matpos[0];pl++)
{
	for(uInt bs=0;bs<NumB;bs++)
	{
		Ants(bs,&a1,&a2);
		if(a1 == a2) continue; // choose cross correlations
		for(int ch=0;ch<matpos[2];ch++)
		{
			for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
			{
				flagc(pl,ch,((tm*NumB+bs))) = flagc(pl,ch,((tm*NumB)+self(a1))) | flagc(pl,ch,((tm*NumB)+self(a1))); 
			}//for tm
		}//for ch
	}//for bs
}//for pl
}

#endif

//cout << " Diagnostics on flag cube " << endl;

float ccnt=0,fcnt=0,pcnt=0;

for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
{
	for(int pl=0;pl<matpos[0];pl++)
	{
		for(int ch=0;ch<matpos[2];ch++)
		{
			for(uInt bs=0;bs<NumB;bs++)
			{
				if( (visc(pl,ch,(((tm*NumB)+bs))) != 0.0) )
				{ 
					pcnt++;
					if( (flagc(pl,ch,((tm*NumB+bs))) == True) ) ccnt++;
					else fcnt++;
				}
			}//for bs
		}//for ch
	}//for pl
}//for tm

//cout << " Percentage Flagged = " << ccnt / (matpos[0]*matpos[2]*cubepos[2]) *100 << endl;
//cout << " Percentage Clear = " << fcnt / (matpos[0]*matpos[2]*cubepos[2]) *100 << endl;

//cout << " Percentage of measured points = " << pcnt / (matpos[0]*matpos[2]*cubepos[2]) *100 << endl;
cout << " Flagged = " << ccnt / pcnt *100 << " %" << endl;
cout << " Clear = " << fcnt / pcnt *100 << " %" << endl;

/* WRITE FLAGS INTO 'flag' for RedFlagger */

//cout << " Writing flags to ChunkFlags " << endl;

/* Use 'corrmask' to indicate which polarizations the flagging applies to */
/* Right now - Flags for only the first polarization, get written */

//cout << "Shape of chunkflags : " << chunkflags.shape() << endl;
//cout << " Nmt : " << Nmt << " NumT : " << NumT << " iterTimecnt : " << iterTimecnt << " NumB : " << NumB << endl;

for(int pl=0;pl<matpos[0];pl++)
{
for(uInt tm=0;tm<Nmt;tm++)
{
	for(uInt bs=0;bs<NumB;bs++)
	{
		//if(RowFlags(pl,(tm*NumB)+bs)==True)
		//	continue;
		for(int ch=0;ch<matpos[2];ch++)
		{
			if(flagc(pl,ch,((tm*NumB)+bs))==True )  
			{
				chunkflags(pl,ch,((tm+iterTimecnt-NumT)*NumB)+bs) = True;
			}
		}//for ch
	}// for bs
}// for tm
}// for pl

timecnt=0;

//flagc = True;

//cout << " Done with this block ! " << endl;
}


iterRowcnt=0;
return RFA::CONT;
}


/* Redflagger call for each ROW - each baseline in a VisBuf */
RFA::IterMode RFATimeFreqCrop :: iterRow  (uInt irow) 
{
	/* DUMMY CALL */
	return RFA::CONT;
}




/*RedFlagger::run - ends loop for all agents with endData 
 * Calls endData once at the end of each PASS */
RFA::IterMode RFATimeFreqCrop :: endData  () 
{
	//cout << " Into End Data " << endl;
	//cout << " End Time Count = " << timecnt << endl;

	pclose(gnu);

	RFAFlagCubeBase::endData();
		return RFA::STOP;
}

/* DUMMY - to override Redflagger's call, which flags everything ! */
void RFATimeFreqCrop :: iterFlag(uInt itime)
{
	//cout << "Set flags for time : " << itime << endl;
	
 flag.advance(itime);
 corrmask = RFDataMapper::corrMask(chunk.visIter());

 const Vector<Int> &ifrs( chunk.ifrNums() );
  if(ant1.shape() != (vb.antenna1()).shape())
	  ant1.resize((vb.antenna1()).shape());
  ant1 = vb.antenna1();
  
  if(ant2.shape() != (vb.antenna2()).shape())
	  ant2.resize((vb.antenna2()).shape());
  ant2 = vb.antenna2();
  uInt npols = (chunkflags.shape())[0];
  uInt nbs = ant1.nelements();
  for(uInt pl=0;pl<npols;pl++)
  {
	  for(uInt ch=StartChan;ch<=EndChan;ch++)
	  {
		  for(uInt bs=0;bs<nbs;bs++)
		  {
			  if(chunkflags(pl,ch-StartChan,(itime*NumB)+baselinecnt))
				flag.setFlag(ch,ifrs(bs));
		  }
	  }
  }

 flag.setMSFlags(itime);
  
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
	RFAFlagCubeBase::endChunk();
	(chunk.visIter()).setRowBlocking(0); //reset to default
//	cout << " Calling endChunk !!" << endl;
}


/* Destructor for RFATimeFreqCrop */

RFATimeFreqCrop :: ~RFATimeFreqCrop () 
{
//    cout << "destructor for RFATimeFreqCrop" << endl;
}



void RFATimeFreqCrop :: UPlot(Vector<Float> vect1, Vector<Float> vect2, uInt lim1, uInt lim2)
{
  //FILE *gnu=NULL;

// uInt choice=0;
// FILE *fp=NULL;
 
 fprintf(gnu,"set xrange [%d:%d]\n",lim1+1+StartChan,lim2+1+StartChan);
 
 Float minv=1000,maxv=0;

 for(uInt i=lim1;i<=lim2;i++)
 {
	minv = (minv<vect1[i])?minv:vect1[i];
	minv = (minv<vect2[i])?minv:vect2[i];
	maxv = (maxv>vect1[i])?maxv:vect1[i];
	maxv = (maxv>vect2[i])?maxv:vect2[i];
 }

 if(minv==maxv) maxv=minv+0.01;

 fprintf(gnu,"set yrange [%f:%f]\n",minv,maxv);
 fprintf(gnu,"set multiplot\n");

  fprintf(gnu,"plot \"-\" u 1:2 w l 1\n");
  for(uInt i=lim1;i<=lim2;i++)
	fprintf(gnu,"%d\t%f\n",i+1,vect1[i]);
  fprintf(gnu,"e\n");
  fflush(gnu);
  
  fprintf(gnu,"plot \"-\" u 1:2 w l 3\n");
  for(uInt i=lim1;i<=lim2;i++)
	fprintf(gnu,"%d\t%f\n",i+1,vect2[i]);
  fprintf(gnu,"e\n");

  fprintf(gnu,"set nomultiplot\n");
  fflush(gnu);
 
//  printf("ps file Y(1),N(0) : ");
//  scanf("%d",&choice);
  
  /*
  if(choice == 1)
  {
   fprintf(gnu,"set term post eps enhanced solid\n");
   fprintf(gnu,"set output \"plot.eps\"\n");
   fprintf(gnu,"replot\n");
   fprintf(gnu,"set term X11\n");
  }
*/
/*
if(choice==1)
{
 fp=fopen("plotdata","w");
 
 fprintf(fp,"set yrange [%f:%f]\n",minv,maxv);
 fprintf(fp,"set multiplot\n");

  fprintf(fp,"plot \"-\" u 1:2 w l 1\n");
  for(uInt i=lim1;i<=lim2;i++)
	fprintf(fp,"%d\t%f\n",i+1,vect1[i]);
  fprintf(fp,"e\n");
  
  fprintf(fp,"plot \"-\" u 1:2 w l 3\n");
  for(uInt i=lim1;i<=lim2;i++)
	fprintf(fp,"%d\t%f\n",i+1,vect2[i]);
  fprintf(fp,"e\n");

  fprintf(fp,"set nomultiplot\n");
 
 fclose(fp);
   cout << " saved to file... plot.eps " << endl; 
   getchar();
}
*/
  cout << "Press enter..." << endl;
   getchar();
  
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
	   for(uInt j=i;j>=0;j--)
		   if(tdata[j]!=0){indl=j;break;}

	   if(indl==-1 && indr==0) tdata[i]=0;
	   if(indl>-1 && indr==0) tdata[i]=tdata[indl];
	   if(indl==-1 && indr>0) tdata[i]=tdata[indr];
	   if(indl>-1 && indr>0) tdata[i]=(tdata[indl]+tdata[indr])/2.0;
      }
   }
  }

  /* replace chan 0 value with chan 1 */
  if(StartChan==0){start=0;tdata[0]=tdata[1];}
  
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
  
  for(uInt j=0;j<=4;j++)
  {
     if(j==0) {deg = 1;npieces=1;}
     if(j==1) {deg = 1;npieces=5;}
     if(j==2) {deg = 2;npieces=6;}
     if(j==3) {deg = 3;npieces=7;}
     if(j==4) {deg = 3;npieces=8;}
     
     psize = (int)(tdata.nelements()/npieces);
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

#ifdef UPLOT 
     UPlot(data,fit,start,data.nelements()-1);	 
#endif
	  
     sd = UStd(tdata,tfband,fit);
     if(j>=2)  TOL=2;
     else TOL=3;
     
     for(uInt i=0;i<tdata.nelements();i++)
     {
	     if(tdata[i]-fit[i] > TOL*sd) 
		     tfband[i]=True;
     }
     
  } // for j
  
} // end of CleanBand

#if 0
void RFATimeFreqCrop :: CleanBand(Vector<Float> data,Vector<Float> fit)
{
  Int step=0,ind=0,deg=0,start=0;
  Int left=0,right=0,le=0,ri=0;
  Float sd,TOL=3;
  Vector<Float> tdata;
  Vector<Bool> tfband;

  tfband.resize(data.nelements());
  tdata.resize(data.nelements());
  
  tfband = False;
  tdata = data;
  

//  Float mean = UMean(data,tfband);
//  if(mean < 10) tdata = tdata * (Float)1e+4;
  
 /* Decide piece size, depending on the actual bandwidth of each channel,
  * and the approximate bandwidth of RFI */
  
  /*
  if(StartChan==0) 
   {tfband[0]=True;start=1;tdata[0]=tdata[1];}
 
  for(uInt i=0;i<data.nelements();i++)
   if(data[i]==0) tfband[i]=True;
   */
  
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
	   for(uInt j=i;j>=0;j--)
		   if(tdata[j]!=0){indl=j;break;}

	   if(indl==-1 && indr==0) tdata[i]=0;
	   if(indl>-1 && indr==0) tdata[i]=tdata[indl];
	   if(indl==-1 && indr>0) tdata[i]=tdata[indr];
	   if(indl>-1 && indr>0) tdata[i]=(tdata[indl]+tdata[indr])/2.0;
      }
   }
  }

  /* replace chan 0 value with chan 1 */
  if(StartChan==0){start=0;tdata[0]=tdata[1];}
  
  /* If there still are empty points (entire spectrum is flagged) flag it. */
  for (uInt i=0;
       i < tdata.nelements();
       i++) {
    
    if(tdata[i]==0) {
      //cout << "chan " << i << " is bland" << endl;
      tfband[i]=True;
    }
  }
  
  fit = tdata;

  //Int PieceSize = int(tdata.nelements()/7.0);
  Int PieceSize = int(tdata.nelements()/7.0);
  
  for(uInt j=0;j<=4;j++)
  {
	  if(j==0) {deg = 1;step=int(tdata.nelements());ind=start;}
	  if(j==1) {deg = 1;step=int(PieceSize*2.0);ind=start;}
	  if(j==2) {deg = 2;step=int(PieceSize*1.5);ind=start;}
	  //if(j==3) {deg = 2;step=int(PieceSize*1.5);ind=start + int(PieceSize*0.25);}
	  if(j==3) {deg = 3;step=int(PieceSize*1.0);ind=start;}
	  if(j==4) {deg = 3;step=int(PieceSize*1.0);ind=start + int(PieceSize*0.25);}
	  //if(j==4) {deg = 7;step=int(tdata.nelements());ind=0;}

	  //if(j==0) {deg = 1;step=20;ind=start;}
	  //if(j==1) {deg = 1;step=15;ind=start;}
	  //if(j==2) {deg = 3;step=10;ind=start+3;}
	  //if(j==3) {deg = 4;step=12;ind=start;}   //// play with this - get rid of 4h order
	  //if(j==4) {deg = 3;step=8;ind=start;}
          
	  /* Check that start,step are consistent with nchans */
	  if(start+step>=(Int)data.nelements()) step=(Int)data.nelements()-1-start;
	 
	 /* first piece */ 
	 le=0;ri=0;
	 
	 if(tfband[ind]==True)
		 while(tfband[le+ind]==True)le++;
	 if(tfband[ind+step]==True)
	 	 while(tfband[ind+step+ri]==True)ri++;	
	
	 left = ind+le;
	 right = ind+step+ri;
	 
	 if(left < 0 ) left = 0;
	 if(right >= (Int)tdata.nelements()) right = tdata.nelements()-1;
	 if(left >= (Int)tdata.nelements() || right <= 0 || (right-left)<3 ) 
	 {  left = 0; right = data.nelements()-1; deg=1; }
	 
	 if(deg==1) 
		 LineFit(tdata,tfband,fit,left,right);
	 else 
	 	PolyFit(tdata,tfband,fit,left,right,deg);
	 ind += step;
 
	 /* middle pieces */
	   while(ind+step < (Int)tdata.nelements())
	   {
	 	le=0;ri=0;

		if(tfband[ind]==True)
			while(tfband[ind+le]==True)le--;	
		
		if(tfband[ind+step]==True)
			while(tfband[ind+step+ri]==True)ri++;	
		
	        left = ind+le;
		right = ind+step+ri;
	 
		if(left < 0 ) left = 0;
		if(right >= (Int)tdata.nelements()) right = tdata.nelements()-1;
		if(left >= (Int)tdata.nelements() || right <= 0 || (right-left)<3 ) 
		{  left = 0; right = data.nelements()-1; deg=1; }
		
	 	if(deg==1)
			 LineFit(tdata,tfband,fit,left,right);
		 else 
		 	PolyFit(tdata,tfband,fit,left,right,deg);
	 	ind += step;
	   }
	 /* last piece */
	 le=0;ri=0;

	if(tfband[ind]==True)
		while(tfband[ind+le]==True)le--;	
	if(tfband[ind+step]==True)
		while(tfband[ind+step+ri]==True)ri--;	
	
	 left = ind-(step/2)+le;
	 right = data.nelements()-1+ri;
		
	 if(left < 0 ) left = 0;
	 if(right >= (Int)tdata.nelements()) right = tdata.nelements()-1;
	 if(left >= (Int)tdata.nelements() || right <= 0 || (right-left)<3 ) 
	 {  left = 0; right = data.nelements()-1; deg=1; }
	 
	 if(deg==1)
		 LineFit(tdata,tfband,fit,left,right);
	 else 	
	 PolyFit(tdata,tfband,fit,left,right,deg);
	
		 
#ifdef UPLOT 
	  UPlot(data,fit,start,data.nelements()-1);	 
#endif
	  
	  sd = UStd(tdata,tfband,fit);
	  if(j>=2)  TOL=2;
	  else TOL=3;
	  
	  for(uInt i=0;i<tdata.nelements();i++)
	  {
		if(tdata[i]-fit[i] > TOL*sd) 
		tfband[i]=True;
	  }
	  

#if 1	  

float jk,max;
int mi,le,ri,cut;
	  
/* flag side chans according to the height of the peak - Sinc Sq response */
	  for (int j = 0; j < 3; j++)
	    {
	      max = 0;
	      mi = 0;
	      for (uInt i = 0; i < tfband.nelements(); i++)
		if (tfband[i] == True)
		  {
		    jk = fabs (tdata[i] - fit[i]);
		    if (jk > max)
		      {
			max = jk;
			mi = i;
		      }
		  }

	     le = mi;
	     ri = mi;
	     cut = 0;		// cut - how much to cut on either side...

/* Cut side chans according to a sinc squared response...
 * Give these numbers according to the channel bandwiths  */

  if (max * 0.05 > sd)
	cut = 2;
      if (max * 0.017 > sd)
	cut = 3;
      if (max * 0.01 > sd)
	cut = 4;
      if (max * 0.005 > sd)
	cut = 5;


	      if (cut > 0)
		{
		  while (le > mi - cut && le > 0)
		    le--;
		  while (ri < mi + cut && ri < (int)(tdata.nelements() - 1))
		    ri++;

		  jk = (tdata[ri] - tdata[le]) / (ri - le);
		  for (int i = le; i <= ri; i++)
		    {
		      tdata[i] = tdata[le] + jk * (i - le);
		      tfband[i] = False;
		    }
		}
	    }			// for j
#endif
	  
  }
  
  //if(mean < 10) fit = fit *(Float)1e-4;
}
#endif


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

   
   if(deg>=0)combination.setCoefficient(0,0.0);
   if(deg>=1)combination.setCoefficient(1,0.0);
   if(deg>=2)combination.setCoefficient(2,0.0);
   if(deg>=3)combination.setCoefficient(3,0.0);
   if(deg>=4)combination.setCoefficient(4,0.0);

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
void RFATimeFreqCrop :: Display_ds9(Int xdim, Int ydim, Float **data, Int frame)
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
      for (i = 0; i < ydim; i++)
	     fwrite (data[i], sizeof (float) * xdim, 1, SAOout);
      pclose (SAOout);
    }
  else
    {
      perror ("Error in opening SAO - ds9 \n");
    }

//cout << " Press enter to continue... " << endl;
//getchar();


}


} //# NAMESPACE CASA - END


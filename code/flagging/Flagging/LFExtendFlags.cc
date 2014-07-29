///# LFExtendFlags.cc: this defines a light autoflagger 
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

#include <flagging/Flagging/LFExtendFlags.h>

#include <sstream>

namespace casa {
  
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
  
  
  // -----------------------------------------------------------------------
  // Default Constructor
  // -----------------------------------------------------------------------
  LFExtendFlags::LFExtendFlags () : LFExamineFlags()
  {
     dbg=True;
     flaglevel_p=0;
     flagcrossfromauto_p=False;
     extendacrosspols_p=False;
  }
  
  LFExtendFlags::~LFExtendFlags ()
  {
  
  }
  
 
  Bool LFExtendFlags::setParameters(Record &parameters)
  {
    /* Create an agent record */
    Record fullrec;
    fullrec = getParameters();
    fullrec.merge(parameters,Record::OverwriteDuplicates);

    //    cout << fullrec << endl;    

    /* Read values into local variables */
    //    flaglevel_p = fullrec.asInt("flag_level");
    //    flagcrossfromauto_p = fullrec.asBool("flag_cross_from_auto");
    flagbothants_p = fullrec.asBool("flag_both_ants");

    extendacrosspols_p = fullrec.asBool("extend_across_pols");

    grow_in_time_p = fullrec.asFloat("grow_in_time");
    grow_in_freq_p = fullrec.asFloat("grow_in_freq");

    if( grow_in_time_p <0.0 || grow_in_time_p >100.0 ) 
      {
	cout << "grow_in_time must be a value between 0.0 and 100.0" << endl;
	return False;
      }
      

    grow_around_p = fullrec.asBool("grow_around");
    flag_prev_next_time_p = fullrec.asBool("flag_prev_next_time");
    flag_prev_next_freq_p = fullrec.asBool("flag_prev_next_freq");
    
    return True;
  }
  
  
  Record LFExtendFlags::getParameters()
  {
    Record rec;
    if( !rec.nfields() )
      {
        rec.define("algorithm","extendflags");
        rec.define("extend_across_pols",False);

	rec.define("grow_in_time",50.0);
	rec.define("grow_in_freq",50.0);
	rec.define("grow_around",True);
	rec.define("flag_prev_next_time",False);
	rec.define("flag_prev_next_freq",False);

	//	rec.define("flag_cross_from_auto",False);
	rec.define("flag_both_ants",False);
      }
    return rec;
  }
  

  //---------------------------------------------------------------------
  // Run all user-chosen extension methods
  //---------------------------------------------------------------------
  Bool LFExtendFlags :: runMethod(const VisBuffer &inVb, Cube<Float> &inVisc, 
				  Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc, 
				   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP)
  {
    // Initialize all the shape information, and make a references for visc and flagc.
    LFExamineFlags::runMethod(inVb, inVisc, inFlagc, inPreFlagc, numT, numAnt, numB, numC, numP);

    if(grow_in_time_p || grow_in_freq_p || grow_around_p || 
       flag_prev_next_freq_p || flag_prev_next_time_p)  GrowFlags();

    if(extendacrosspols_p) ExtendFlagsAcrossPols();    

    //    if(flagcrossfromauto_p) FlagBaselinesFromAntennas();
    //    if(flagbothantennas_p) FlagAntennasFromBaselines();

    //    if(extendfields_p || extendspw_p || extendtime_p) makeFlagCmds();

    return True;   
  }// End of runMethod()
  
  
   
  
  //---------------------------------------------------------------------- 
  // /* Grow Flags based on surrounding values */
  //---------------------------------------------------------------------- 
  void LFExtendFlags :: GrowFlags()
  {
    uInt a1,a2;
    // For all correlation pairs.....
    for(int pl=0;pl<(int) NumP;pl++)
      {
	// For all baselines...
	for(uInt bs=0;bs<NumB;bs++)
	  {
	    Ants(bs,&a1,&a2);
	    if( (a1 == a2) ||  (baselineFlag[bs]==False) ) continue; // choose cross correlations
	    
	    // For all channels....
	    for(int ch=0;ch<(int) NumC;ch++)
	      { 
		// For all timesteps....
		for(uInt tm=0;tm<NumT;tm++)
		  {       
		    // flag one timestamp before and after
		    if(flag_prev_next_time_p)
		      {
			if(tm>0)
			  if(flagc(pl,ch,((tm*NumB+bs)))==True)
			    flagc(pl,ch,(((tm-1)*NumB+bs)))=True;
			
			if((NumT-tm)<NumT-1)
			  if(flagc(pl,ch,(((NumT-tm)*NumB+bs)))==True)
			    flagc(pl,ch,(((NumT-tm+1)*NumB+bs)))=True;
		      }// end flag_prev_next_time_p
		    
		    // flag one channel before and after
		    if(flag_prev_next_freq_p) 
		      {
			if(ch>0)
			  if(flagc(pl,ch,((tm*NumB+bs)))==True)
			    flagc(pl,ch-1,((tm*NumB+bs)))=True;
			
			if((NumC-ch)<NumC-1)
			  if(flagc(pl,(NumC-ch),(tm*NumB+bs))==True)
			    flagc(pl,(NumC-ch+1),(tm*NumB+bs))=True;
		      }// end flag_prev_next_freq_p
		    
		    if(grow_around_p) 
		      {
			/* If previous and next channel are flagged, flag it */
			if(ch>0 && ch < (int) NumC-1)
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
		      }// end grow_around_p

		  }//for tm
	      }//for ch
	    
	    if(grow_around_p) 
	      {
		/* If more than 4 surrounding points are flagged, flag it */
		uInt fsum2=0;
		for(int ch=1;ch<(int) NumC-1;ch++)
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
	      }// if grow_around_p
	    
	    if(grow_in_time_p>0.0)
	      {
		uInt fsum2=0;
		
		/* Grow flags in time - if more than X% of the timerange is flagged*/
		for(int ch=0;ch<(int) NumC;ch++)
		  { 
		    fsum2=0;
		    /* count unflagged points for this channel (all times) */
		    for(uInt tm=0;tm<NumT;tm++)
		      fsum2 += (flagc(pl,ch,((tm*NumB+bs)))==True)?0:1 ; 
		    /*if more than 50% of the timetange flagged - flag whole timerange for that channel */
		    if(fsum2 < (1.0 - (grow_in_time_p/100.0))*NumT)
		      for(uInt tm=0;tm<NumT;tm++)
			flagc(pl,ch,((tm*NumB+bs)))=True;
		  }// for ch
		
	      }// end grow_in_time_p

	    if(grow_in_freq_p>0.0)
	      {
		/* Grow flags in frequency - if more than X% of the freq-range is flagged*/
		for(int tm=0;tm<(int) NumT;tm++)
		  { 
		    uInt fsum2=0;
		    /* count unflagged points for this channel (all times) */
		    for(uInt ch=0;ch<NumC;ch++)
		      fsum2 += (flagc(pl,ch,((tm*NumB+bs)))==True)?0:1 ; 
		    /*if more than 50% of the freq-range flagged - flag all channels for that timestep */
		    if(fsum2 < (1.0 - (grow_in_freq_p/100.0))*NumC)
		      for(uInt ch=0;ch<NumC;ch++)
			flagc(pl,ch,((tm*NumB+bs)))=True;
		  }// for ch
	      }// end grow_in_freq_p

	  }// end of bs
      }// end of pl
    
  }// end of GrowFlags
  
  
  //---------------------------------------------------------------------
  // If autocorrelations are used for finding RFI, apply flags to cross-correlations
  //---------------------------------------------------------------------
  void LFExtendFlags :: FlagBaselinesFromAntennas()
  {    
    uInt a1,a2;
    //    cout << " Flagging Cross correlations from self correlation flags " << endl;
    for(int pl=0;pl<(int) NumP;pl++)
      {
	for(uInt bs=0;bs<NumB;bs++)
	  {
	    Ants(bs,&a1,&a2);
	    if(a1 == a2) continue; // choose cross correlations
	    for(int ch=0;ch<(int) NumC;ch++)
	      {
		for(uInt tm=0;tm<NumT;tm++)
		  {
		    flagc(pl,ch,((tm*NumB+bs))) = flagc(pl,ch,((tm*NumB)+Self(a1))) | flagc(pl,ch,((tm*NumB)+Self(a1))); 
		  }//for tm
	      }//for ch
	  }//for bs
      }//for pl
    
  }// end FlagCorrsFromSelfs
  
  
  
  //---------------------------------------------------------------------- 
  // /* Extend flags across polarization */
  //---------------------------------------------------------------------- 
  void LFExtendFlags :: ExtendFlagsAcrossPols()
  {
    uInt a1,a2;
    
    for(uInt bs=0;bs<NumB;bs++)
      {
	Ants(bs,&a1,&a2);
	if(a1 == a2) continue; // choose cross correlations
	for(int ch=0;ch<(int)NumC;ch++)
	  {
	    for(uInt tm=0;tm<NumT;tm++)
	      {
		Bool polflag = False;
		for(int pl=0;pl<(int) NumP;pl++)  { polflag |=  flagc(pl,ch,((tm*NumB+bs)));  }//for pl
		for(int pl=0;pl<(int) NumP;pl++)  { flagc(pl,ch,((tm*NumB+bs))) = polflag;  }//for pl
	      }//for tm
	  }//for ch
      }//for bs
    
  } 
  
} //#end casa namespace

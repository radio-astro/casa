///# LFExamineFlags.cc: this defines a light autoflagger 
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

#include <flagging/Flagging/LFExamineFlags.h>

#include <sstream>
#include <iostream>
#include <iomanip>

namespace casa {
  
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
  
  // -----------------------------------------------------------------------
  // Default Constructor
  // -----------------------------------------------------------------------
  LFExamineFlags::LFExamineFlags () : LFBase()
  {
    allflagcounts.define("method","displayflags");
    allflags.clear();
    allcounts.clear();
    antnames_p.resize();
    corrlist_p.resize();
}


  LFExamineFlags::~LFExamineFlags ()
  {}
  
  
  Bool LFExamineFlags::setParameters(Record &parameters)
  {
    /* Create an agent record */
    Record fullrec;
    fullrec = getParameters();
    fullrec.merge(parameters,Record::OverwriteDuplicates);
    
     /* Read values into local variables */
    // None yet. 
    
    return True;
  }
  
  
  Record LFExamineFlags::getParameters()
  {
    Record rec;
    if( !rec.nfields() )
      {
        rec.define("algorithm","examineflags");
      }
    return rec;
  }
  
  
  /* Extend Flags */
  /* Openmp on baselines... */
  Bool LFExamineFlags :: runMethod(const VisBuffer &inVb, 
				  Cube<Float> &inVisc, Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc,
				   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP)
  {
    LogIO os = LogIO(LogOrigin("ExamineFlags","runMethod()",WHERE));
    // Initialize all the shape information, and make a references for visc and flagc.
    LFBase::runMethod(inVb, inVisc, inFlagc, inPreFlagc, numT, numAnt, numB, numC, numP);
    
    // Read MS info from the vb
    if(antnames_p.nelements()==0) ReadMSInfo(inVb);
   
     // Initialize various counters
    uInt a1,a2;
    IPosition shp(2),tshp(2); shp(0)=NumC; shp(1)=NumT;
    chan_count.resize(NumC); chan_count=0;
    chan_flags.resize(NumC); chan_flags=0;
    baseline_count.resize(NumB); baseline_count=0;
    baseline_flags.resize(NumB); baseline_flags=0;
    corr_count.resize(NumP); corr_count=0;
    corr_flags.resize(NumP); corr_flags=0;
    chunk_count=0; chunk_flags=0;


    AlwaysAssert( NumP == corrlist_p.nelements() , AipsError );
    AlwaysAssert( NumB == baselineFlag.nelements() , AipsError );

    // Iterate through the full chunk of data    
    for(Int bs=0;bs<(Int)NumB;bs++) // Start Baseline Loop
      {
        AlwaysAssert(bs>=0 && bs<(Int) NumB, AipsError);
	Ants(bs,&a1,&a2);
	AlwaysAssert( /*a1>=0 && */ a1<antnames_p.nelements(), AipsError );
	AlwaysAssert( /* a2>=0 && */ a2<antnames_p.nelements(), AipsError );
	//cout << "baseline : " << bs << " ants : " << a1 << "," << a2 << endl;
	if( (a1 != a2)  &&  baselineFlag[bs]==True) // If only cross-correlations, and if baseline exists in data
	  {
	    for(int pl=0;pl<(int)NumP;pl++)  // Start Correlation Loop
	      {
		for(int ch=0;ch<(int)NumC;ch++)  // Start Channel Loop
		  { 
		    for(uInt tm=0;tm<NumT;tm++)  // Start Time Loop
		      {       
                          chan_count[ch]++;   baseline_count[bs]++;  
			  corr_count[pl]++; chunk_count++;
                          if( flagc(pl,ch,(tm*NumB)+bs) ) 
			    {
			      chan_flags[ch]++; baseline_flags[bs]++; 
			      corr_flags[pl]++; chunk_flags++;
			    }
		      }// End Time Loop
		  }//End Channel Loop
	      }//End Correlation Loop
	  }// end if cross-correlations only
      }//End Baseline Loop

    AccumulateStats(inVb);

    return True;
  }// end runMethod
  

/************************************************************************/
  // Display final Stat counts on the plotter 
  // Construct a record of counts by mapping to visbuffer indices...
  // and return the record of counts.
  /************************************************************************/
  Record LFExamineFlags :: getStatistics()
  {
    Record res;

    for (map<string, map<string, float> >::iterator j = allcounts.begin();
           j != allcounts.end();
           j++) {
        
          Record prop;
          for (map<string, float>::const_iterator i = j->second.begin();
               i != j->second.end();
               i++) {
            
              Record tmp;

              tmp.define("flagged", (uInt) allflags[j->first][i->first]);
              tmp.define("total", (uInt) i->second);
              
              prop.defineRecord(i->first, tmp);
          }
          
          res.defineRecord(j->first, prop);
      }

    return res;
  }

  /***************************************************************/
  /***************************************************************/
  /***************************************************************/
  void LFExamineFlags::AccumulateStats(const VisBuffer &vb)
  {
   LogIO os = LogIO(LogOrigin("ExamineFlags","Stats()",WHERE));

   //if(chunk_count>0)  os << LogIO::NORMAL << " --> Flagged " << 100*chunk_flags/chunk_count << "% ";
   //else os << LogIO::NORMAL << " --> No data to flag" ;
    
    // Read info from the vb.
    Int spw=vb.spectralWindow();
    Int fieldid = vb.fieldId();

    // Accumulate field counts
    stringstream fieldstr;
    fieldstr << fieldid ;
    allflags["field"][fieldstr.str()] += chunk_flags;
    allcounts["field"][fieldstr.str()] += chunk_count;
    
    // Accumulate spw counts
    stringstream spwstr;
    spwstr << spw ;
    allflags["spw"][spwstr.str()] += chunk_flags;
    allcounts["spw"][spwstr.str()] += chunk_count;
    
    // Accumulate channel counts
    for(int ch=0;ch<(int) NumC;ch++)
      { 
	stringstream chanstr;
	chanstr << fieldid << ":" << spw << ":" << ch;
	allflags["channel"][chanstr.str()] += chan_flags[ch];
	allcounts["channel"][chanstr.str()] += chan_count[ch];
      }
    
    // Accumulate baseline counts    
    for(uInt bs=0;bs<NumB;bs++) 
      {
	Ants(bs,&a1,&a2);
	if(a1 != a2) // If only cross-correlations
	  {
	    stringstream baselinestr;
	    baselinestr << fieldid << "-" << spw << "-" << a1 << "-" << a2;
	    allflags["baseline"][baselinestr.str()] += baseline_flags[bs];
	    allcounts["baseline"][baselinestr.str()] += baseline_count[bs];
	  }
      }
    
    // Accumulate correlation counts    
    for(int pl=0;pl<(int) NumP;pl++)
      {
	stringstream corrstr;
	corrstr << spw << ":" << pl;
	allflags["correlation"][corrstr.str()] += corr_flags[pl];
	allcounts["correlation"][corrstr.str()] += corr_count[pl];
	//if(corr_count[pl]>0) 	os << " [" << corrlist_p[pl] << "]:" << 100*corr_flags[pl]/corr_count[pl] ;
	//else os << " [" << corrlist_p[pl] << "]: No data" ;
      }    

    //os << LogIO::POST;

  }// end of accumulateStats
  /***************************************************************/
  /***************************************************************/

  void LFExamineFlags::ReadMSInfo(const VisBuffer &vb)
  {
    // ROMSColumns msc = vb.msColumns();
    //ROMSAntennaColumns antcol = msc.antenna();
    //ROScalarColumn antnamecol = antcol.name();
    //antnames_p = antnamecol.getColumn();

    antnames_p = vb.msColumns().antenna().name().getColumn();

    Vector<Int> corrtypes;
    vb.msColumns().polarization().corrType().get(0,corrtypes);

    corrlist_p.resize(corrtypes.nelements());
    for(Int i=0;i<(int) corrtypes.nelements();i++)
        corrlist_p[i] = Stokes::name((Stokes::StokesTypes)corrtypes[i]);

    fieldnames_p = vb.msColumns().field().name().getColumn();

  }

  /***************************************************************/
  /***************************************************************/
 
  
} //#end casa namespace

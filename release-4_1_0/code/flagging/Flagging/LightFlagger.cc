///# LightFlagger.cc: this defines a light autoflagger 
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
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <casa/BasicSL/Complex.h>
#include <measures/Measures/Stokes.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/HostInfo.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/System/ProgressMeter.h>
#include <casa/stdio.h>
#include <casa/math.h>
#include <stdarg.h>

#include <tables/Tables/ExprNode.h>
//#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/VisSetUtil.h>

#include <measures/Measures/Stokes.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/MVEpoch.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>

#include <flagging/Flagging/LightFlagger.h>
#include <flagging/Flagging/LFTimeFreqCrop.h>
#include <flagging/Flagging/LFExtendFlags.h>
#include <flagging/Flagging/LFDisplayFlags.h>
//#include <flagging/Flagging/LFExamineFlags.h>

#include <sstream>

namespace casa {
  
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
  
  // -----------------------------------------------------------------------
  // Default Constructor
  // -----------------------------------------------------------------------
  LightFlagger::LightFlagger () : ms_p(),mssel_p(),mss_p(), vs_p(0)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","constructor without ms",WHERE));
    dbg=False;
    if(dbg) os<< "LightFlagger constructor" << LogIO::POST;

    maxnumthreads_p = 1;
    //    maxnumthreads_p = 4;

  }
  
  // -----------------------------------------------------------------------
  // Default Destructor
  // -----------------------------------------------------------------------
  LightFlagger::~LightFlagger ()
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","destructor",WHERE));
    if(dbg) os<< "LightFlagger :: destructor" << LogIO::POST;
    
    if(vs_p) delete vs_p;
    
  }
  
  // -----------------------------------------------------------------------
  // Connect to an MS. 
  //  TODO : Use this function to make the flagger re-entrant from the tool level
  // -----------------------------------------------------------------------
  Bool LightFlagger:: attach (String msname)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","attach()",WHERE));
    os << "Attaching to measurementset : " << msname << LogIO::POST;
    
    ms_p = MeasurementSet(msname,Table::Update);
    
    /* check the MS */
    if (ms_p.isNull()) {
      os << LogIO::SEVERE << "NO MeasurementSet attached" << LogIO::POST;
      return False;
    }
    
    return True;
  }
  
  
  // -----------------------------------------------------------------------
  //  Data selection : Use MS-Selection to get a reference sub-ms
  // -----------------------------------------------------------------------
  Bool LightFlagger::setdata(String field, String spw, String array,
			     String feed, String scan,
			     String baseline, String uvrange, String time,
			     String correlation)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","setdata",WHERE));
    os << "setdata: " 
		<< " field=" << field << " spw=" << spw
		<< " array=" << array << " feed=" << feed
		<< " scan=" << scan << " baseline=" << baseline
		<< " uvrange=" << uvrange << " time=" << time
		<< " correlation=" << correlation << LogIO::POST;
    
    /* check the MS */
    if (ms_p.isNull()) {
      os << LogIO::SEVERE << "NO MeasurementSet attached"
	 << LogIO::POST;
      return False;
    }
    
    mss_p.reset(ms_p,MSSelection::PARSE_NOW,time, baseline, field, spw, uvrange, "", correlation, scan, array, "");
    
    mss_p.getSelectedMS(mssel_p, String(""));
    
    os << "Original ms has nrows : " << ms_p.nrow() << LogIO::POST;
    os << "Selected ms has nrows : " << mssel_p.nrow() << LogIO::POST;
    
    return True;
  }
  
  
  // -----------------------------------------------------------------------
  //  Build up a list of agents, and set their parameters
  //  TODO : Add support for more agent types, as they are implemented
  // -----------------------------------------------------------------------
  Bool LightFlagger::setparameters(String algorithm, Record &parameters)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","setparameters",WHERE));
    if(dbg) os<< "LightFlagger :: setparameters" << LogIO::POST;
    /*
      if (ms.isNull()) {
      os << LogIO::SEVERE << "NO MeasurementSet attached"
      << LogIO::POST;
      return False;
      }
    */    

    /* Create an agent record */
    Int nmethods = flagmethods_p.nelements();
    Bool ret=True;

    if(algorithm == "tfcrop")
      {
	flagmethods_p.resize(nmethods+1,True); 
	flagmethods_p[nmethods] = new LFTimeFreqCrop();
	ret = flagmethods_p[nmethods]->setParameters(parameters);
      }
    else if(algorithm == "extendflags")
      {
	flagmethods_p.resize(nmethods+1,True); 
	flagmethods_p[nmethods] = new LFExtendFlags();
	ret = flagmethods_p[nmethods]->setParameters(parameters);
      }
    else
      {
	os << LogIO::WARN << "Un-recognized method : " << algorithm << LogIO::POST;
	return False;
      }

    if(ret==False)
      {
	os << LogIO::WARN << "Input parameters for [" << algorithm << "] are invalid" << LogIO::POST;
	flagmethods_p.resize(nmethods,True);
      }

    //ostringstream oss (ostringstream::out);
    //parameters.print(oss);
    //os << "Added method : " << algorithm << " with parameters : " << oss.str() <<  LogIO::POST ;
    
    return True;
  }
  
  // -----------------------------------------------------------------------
  // Get default parameters for the requested agent
  // This is for the tool user to know what params are available
  // TODO : Add support for other agent types as they are implemented
  // -----------------------------------------------------------------------
  Record LightFlagger::getparameters(String algorithm)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","getparameters",WHERE));
    if(dbg) os<< "LightFlagger :: getparameters" << LogIO::POST;
    
    Record rec;
    
    if(algorithm == "tfcrop")
      {
	LFTimeFreqCrop tmp = LFTimeFreqCrop();
        rec = tmp.getParameters();
      }
    else if(algorithm == "extendflags")
      {
	LFExtendFlags tmp = LFExtendFlags();
        rec = tmp.getParameters();
      }
    else
      {
	os << LogIO::WARN << "Un-recognized method : " << algorithm << LogIO::POST;
      }
  
    
    return rec;
  }

  //-----------------------------------------------------------------------  
  // Default parameters for LightFlagger. 
  // These parameters control global operations.
  // TODO : the "column" and "expr" parameters need to move into the agents
  //-----------------------------------------------------------------------  
  Record LightFlagger::defaultParameters()
  {
    Record rec;
    if( !rec.nfields() )
      {
	rec.define("ntime_sec",90);
	rec.define("column","DATA");
	rec.define("expr","ABS I");
	rec.define("fignore",False);
	rec.define("showplots",False);
	rec.define("flag_level",1);
        rec.define("writeflags",False);
        rec.define("flagzeros",False);
	rec.define("usepreflags",True);
      }
    return rec;
  }
  
  //-----------------------------------------------------------------------  
  // Fill internal variables with the input parameters.
  // This is called from run()
  //-----------------------------------------------------------------------  
   void LightFlagger::initParameters(Record &parameters)
  {
    Record genpar = defaultParameters();
    genpar.merge(parameters, Record::OverwriteDuplicates);

    NumTime = genpar.asInt("ntime_sec");
    ShowPlots = genpar.asBool("showplots");
    FlagLevel = genpar.asInt("flag_level");
    WriteFlagsToMS = genpar.asBool("writeflags");
    FlagZeros = genpar.asBool("flagzeros");
    UsePreFlags = genpar.asBool("usepreflags");
    //Expr = genpar.asString("expr");
    Column = genpar.asString("column");

    // Initialize other parameters
    StopAndExit=False;

  }
  // -----------------------------------------------------------------------
  // LightFlagger::run
  // Iterate through the MS (for all chunks...)
  //     Read visibilities and flags 
  //     Call all agents::runMethod
  //     Call display/examiner agent
  // -----------------------------------------------------------------------
  Record LightFlagger::run(Record &parameters) 
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","run()",WHERE));
    if(dbg) os<< "LightFlagger :: run()" << LogIO::POST;
    
    /* check the MS */
    if (mssel_p.isNull()) {
      os << LogIO::SEVERE << "No selected MeasurementSet attached" << LogIO::POST;
      return Record();
    }
    
    // Initialize Parameters
    initParameters(parameters);
    
    // Setup Display/Statistics Method
    LFDisplayFlags flagstats;
    flagstats.setParameters(parameters);
    
    // Setup visibility iterator : vs_p
    setupIterator();
    
    // Iterate over the selected MS.    
    VisibilityIterator &vi(vs_p->iter()); 
    vi.slurp();
    VisBuffer vb(vi);
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) 
      {
        // Gather all shapes for this chunk
        VisCubeShp = vi.visibilityShape();
        NumT = vi.nSubInterval();
	Nrow = vi.nRowChunk();
        NumC = vb.nChannel(); //VisCubeShp[1]; 
        Ncorr = vb.nCorr(); //VisCubeShp[0]; 
	NumAnt = vi.numberAnt();
	NumB = ((NumAnt)*((NumAnt)-1)/2 + (NumAnt));
	
        NumP = Ncorr;
	
	os << "Field=" << vi.fieldId() << " Name=" << vi.fieldName() << " Spw=" << vi.spectralWindow() << " N_ant=" << NumAnt << " N_timesteps=" << NumT << "  N_chan=" << NumC << "  N_corr=" << NumP << LogIO::POST;

        	
        // Resize working arrays : NumP x NumC x NumB x NumT
        AllocateMemory();
	
        // Gather flags into 'visc, flagc and preflagc' for all timesteps in the chunk
        uInt timecnt=0;
	for( vi.origin(); vi.more(); vi++) 
	  {
	    readVisAndFlags(vb,timecnt);
            timecnt++;
	  }
	AlwaysAssert(timecnt == NumT, AipsError);
	
        // Run all the flagging methods in sequence (except for the display)
	for(Int i=0; i<(Int) flagmethods_p.nelements(); i++)
	  {
	    AlwaysAssert(!flagmethods_p[i].null(), AipsError);
	    //os << "Running method : " << flagmethods_p[i]->methodName() << LogIO::POST;
	    flagmethods_p[i]->setBaselineFlag(baselineFlag);
	    flagmethods_p[i]->runMethod(vb, visc, flagc, preflagc, NumT, NumAnt, NumB, NumC, NumP);
	  }
	
        // Display Flags	
	flagstats.setBaselineFlag(baselineFlag);
        Bool ret = flagstats.runMethod(vb, visc, flagc, preflagc, NumT, NumAnt, NumB, NumC, NumP, flagmethods_p);
        if(ret==False) return Record();
	
        // Write flags into the flagcube.
        if(WriteFlagsToMS)
	  {
	    timecnt=0;
	    for( vi.origin(); vi.more(); vi++) 
	      {
		writeFlags(vi,vb,timecnt);
		timecnt++;
	      }
	    AlwaysAssert(timecnt == NumT, AipsError);
	  }
	
      }// end loop over chunks
    
    // Get the statistics record to return...
    Record allcounts = flagstats.getStatistics();    
    
    //ostringstream oss (ostringstream::out);
    //allcounts.print(oss);
    //os << "STATISTICS : " << oss.str() <<  LogIO::POST ;
    // cout << "STATISTICS : " << oss.str() << endl;
    
    // Clean up the method list
    flagmethods_p.resize(0);
    
    return allcounts;
  }
  
  /***********************************************************/  
  //-----------------------------------------------------------------------  
  // Set up visibility iterator with the user-supplied time-interval and channel-selection,
  //-----------------------------------------------------------------------  
  Bool LightFlagger::setupIterator()
  {
    Matrix<Int> noselection;
    Block<int> sort(4);
    //sort2[0] = MS::SCAN_NUMBER;
    // Do scan priority only if quacking
    sort[0]= MS::ARRAY_ID;
    sort[1]= MS::FIELD_ID;
    sort[2]= MS::DATA_DESC_ID;
    sort[3] = MS::TIME;
    
    Double timeInterval = NumTime ;
    
    if (mssel_p.isNull())
      throw AipsError("No measurement set selection available");
    
    Bool addScratch = False;
    if ( vs_p ) delete vs_p;
    vs_p = new VisSet(mssel_p, sort, noselection, addScratch, timeInterval);
    vs_p->resetVisIter(sort, timeInterval);
    
    
    // If there is channel selection, set this up explicitly.
    Vector<Int> spwlist = mss_p.getSpwList();
    
    if ( spwlist.nelements()) 
      {
	Matrix<Int> spwchan = mss_p.getChanList();
	IPosition cshp = spwchan.shape();
	if ( (Int)spwlist.nelements() > (spwchan.shape())[0] )
	  cout << "WARN : Using only the first channel range per spw" << endl;
	for( uInt i=0;i<spwlist.nelements();i++ )
	  {
	    Int j=0;
	    for ( j=0;j<(spwchan.shape())[0];j++ )
	      if ( spwchan(j,0) == spwlist[i] ) break;
	    vs_p->iter().selectChannel(1, Int(spwchan(j,1)), 
				       Int(spwchan(j,2)-spwchan(j,1)+1),
				       Int(spwchan(j,3)), spwlist[i]);
	  }
      }
    
    // Ignore correlation selection (because this is not shown to the user)
    // All corrs are always read in, but the user can control which are flagged on
    //   - this is to allow flag extensions across pols.
    
    return True;
  }

  //-----------------------------------------------------------------------  
  // Data Handler : read visibilities and flags from visbuffers
  //                        into local arrays covering the full chunk size.
  // These are the arrays that pass into agent.runMethod() by reference
  //
  // TODO : Move this function into a data-handler class
  // TODO : Provide data-mapper functionality
  // TODO : Provide data-access functions so that each agent could read a different
  //             type of data ( abs(data), phase(corrected_data), etc... )
  //             => Agents will read the data individually, but flags are common to all agents
  //             => However, all vi++ loops should stay within LightFlagger if possible (out of the agents)
  //-----------------------------------------------------------------------  
  Bool LightFlagger:: readVisAndFlags(VisBuffer &vb, uInt timecnt)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","readVisAndFlags()",WHERE));
    ant1.resize( (vb.antenna1()).shape() ); ant1 = vb.antenna1();
    ant2.resize( (vb.antenna2()).shape() ); ant2 = vb.antenna2();
    uInt nrows = vb.nRow();
    AlwaysAssert( (nrows==ant1.nelements() && nrows==ant2.nelements()), AipsError );
    Cube<Bool> flagcube(vb.flagCube());
    // replace with a function that returns a Float for the resulting math operation and column.
    //Cube<Complex> viscube(vb.visCube());
    Cube<Complex> viscube;
    
    if(upcase(Column)=="DATA") viscube.reference(vb.visCube());
    else if(upcase(Column)=="MODEL") viscube.reference(vb.modelVisCube());
    else if(upcase(Column)=="CORRECTED") viscube.reference(vb.correctedVisCube());
    else if(upcase(Column)=="RESIDUAL") viscube.assign(vb.correctedVisCube() - vb.modelVisCube());
    else if(upcase(Column)=="RESIDUAL_DATA") viscube.assign(vb.visCube() - vb.modelVisCube());
    else 
      {
	os << LogIO::WARN << "Cannot recognize " << Column << "  :  Using DATA column : " << LogIO::POST;
	Column = String("DATA");
	viscube.reference(vb.visCube());
      }
    
        
    AlwaysAssert( VisCubeShp[0] == viscube.shape()[0] , AipsError);
    AlwaysAssert( VisCubeShp[1] == viscube.shape()[1] , AipsError);
    AlwaysAssert( viscube.shape()[2] <= NumB , AipsError);

    // Make a copy of the original flags - to compare before/after statistics. Fill this also
    //preflagc.assign(flagc);
    
    uInt baselineindex = 0, rowindex=0;
    for(uInt pl=0;pl<NumP;pl++)
      {
	// Iterate through rows in the visbuffer, filling data and flags
	for(uInt row=0;row<nrows;row++)
	  {
            baselineindex = BaselineIndex(row,ant1[row],ant2[row]);
	    baselineFlag[baselineindex]=True;// this baseline is present in the data
	    for(uInt ch=0;ch<NumC;ch++)
	      {
                rowindex = (timecnt*NumB)+baselineindex;
                // Fill data
 	        visc(pl,ch,rowindex) = fabs(viscube(pl,ch,row));
		// Fill flags
		flagc(pl,ch,rowindex) = flagcube(pl,ch,row);
	      }//for ch
	  }//for row

	// Re-iterate through rows, this time as NumT and NumB and then NumC
	// Flag Zeros
	// Needs to be done here..... to account for visbuffer-rows with no data for only some baselines...
	for(uInt bs=0;bs<NumB;bs++)
	  {
	    for(uInt tm=0;tm<NumT;tm++)
	      {
		rowindex = (((tm*NumB)+bs));
		for(uInt ch=0;ch<NumC;ch++)
		  {
		    if(UsePreFlags==False) flagc(pl,ch,rowindex)=False; // Overkill here....
		    if(FlagZeros && visc(pl,ch,rowindex)<(Float)1e-08) flagc(pl,ch,rowindex)=True;
		  }// for ch
	      }//for tm
	  }//for bs
      }//for pl

    // Record pre-flags for stats calculations later on.
    preflagc.assign(flagc);
    
    return True;
  }
  
  //-----------------------------------------------------------------------  
  // Write flags to the chunk (all visbuffers in it).
  // The local flag-cube has been modified by all agents by this time.
  //-----------------------------------------------------------------------  
  Bool LightFlagger:: writeFlags(VisibilityIterator &vi, VisBuffer &vb, uInt timecnt)
  {
    ant1.resize( (vb.antenna1()).shape() ); ant1 = vb.antenna1();
    ant2.resize( (vb.antenna2()).shape() ); ant2 = vb.antenna2();
    uInt nrows = vb.nRow();
    AlwaysAssert( (nrows==ant1.nelements() && nrows==ant2.nelements()), AipsError );
    Cube<Bool> flagcube(vb.flagCube());
    
    AlwaysAssert( VisCubeShp[0] == (flagcube.shape())[0] , AipsError);
    AlwaysAssert( VisCubeShp[1] == (flagcube.shape())[1] , AipsError);
    AlwaysAssert( (flagcube.shape())[2] <= NumB , AipsError);
    
    uInt baselineindex=0;
    Int countflags=0, countpnts=0;
    for(uInt pl=0;pl<NumP;pl++)
      {
	for(uInt row=0;row<nrows;row++)
	  {
            baselineindex = BaselineIndex(row,ant1[row],ant2[row]);
	    for(uInt ch=0;ch<NumC;ch++)
	      {
		flagcube(pl,ch,row) = flagc(pl,ch,(timecnt*NumB)+baselineindex);
		countpnts++;
		countflags += Int( flagcube(pl,ch,row) );
	      }
	  }
      }
    vi.setFlag(flagcube);
    
    //    if(countflags>0) cout << "Time : " << timecnt << " Postflags : " << countflags << " out of " << countpnts << endl;
    
    
    return True;
  }
  
  
  //-----------------------------------------------------------------------  
  // Resize cubes to hold data and flags (preflags is not listed here)
  //-----------------------------------------------------------------------  
  void LightFlagger :: AllocateMemory()
  {
    
    /* Cube to hold visibility amplitudes : POLZN x CHAN x (IFR*TIME) */
    visc.resize(NumP,NumC,NumB*NumT);
    visc=0;
    
    /* Cube to hold visibility flags : POLZN x CHAN x (IFR*TIME) */
    flagc.resize(NumP,NumC,NumB*NumT);
    flagc=False;

    //preflagc.resize(NumP,NumC,NumB*NumT);
    //preflagc=False;
    // 	cout << " CubeShape = " << cubepos << endl;

    baselineFlag.resize(NumB); baselineFlag=False; // all baselines are absent from the data

  }
  
  
  //-----------------------------------------------------------------------  
  //  /* Return antenna numbers from baseline number - upper triangle storage */
  //-----------------------------------------------------------------------  
  void LightFlagger :: Ants(uInt bs, uInt *a1, uInt *a2)
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
  
  //-----------------------------------------------------------------------  
  //  /* Return baseline index from a pair of antenna numbers - upper triangle storage */
  //-----------------------------------------------------------------------  
  uInt LightFlagger :: BaselineIndex(uInt /*row*/, uInt a1, uInt a2)
  {
    return ( (NumAnt)*((NumAnt)+1)/2 - ((NumAnt)-a1)*((NumAnt)-a1+1)/2 + (a2 - a1) );
  }
  
  
  
  /* FLAG VERSION SUPPORT */ 
  //-----------------------------------------------------------------------  
  // Save Flag Version
  //-----------------------------------------------------------------------  
  Bool LightFlagger::saveFlagVersion(String versionname, String comment, String merge )
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","saveflagversion",WHERE));
    try
      {
	FlagVersion fv(ms_p.tableName(),"FLAG","FLAG_ROW");
	fv.saveFlagVersion(versionname, comment, merge);
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not save Flag Version : " << x.getMesg() << LogIO::POST;
	throw;
      }
    return True;
  }
  
  //-----------------------------------------------------------------------  
  // Restore Flag Version
  //-----------------------------------------------------------------------  
  Bool LightFlagger::restoreFlagVersion(Vector<String> versionname, String merge )
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","restoreflagversion",WHERE));
    try
      {
	FlagVersion fv(ms_p.tableName(),"FLAG","FLAG_ROW");
	for(Int j=0;j<(Int)versionname.nelements();j++)
	  fv.restoreFlagVersion(versionname[j], merge);
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not restore Flag Version : " << x.getMesg() << LogIO::POST;
	return False;
      }
    return True;
  }
  
  //-----------------------------------------------------------------------  
  // Delete Flag Version
  //-----------------------------------------------------------------------  
  Bool LightFlagger::deleteFlagVersion(Vector<String> versionname)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","deleteflagversion",WHERE));
    try
      {
	FlagVersion fv(ms_p.tableName(),"FLAG","FLAG_ROW");
	for(Int j=0;j<(Int)versionname.nelements();j++)
	  fv.deleteFlagVersion(versionname[j]);
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not delete Flag Version : " << x.getMesg() << LogIO::POST;
	return False;
      }
    return True;
  }
  
  //-----------------------------------------------------------------------  
  // Get Flag Version List
  //-----------------------------------------------------------------------  
  Bool LightFlagger::getFlagVersionList(Vector<String> &verlist)
  {
    LogIO os = LogIO(LogOrigin("LightFlagger","getflagversionlist",WHERE));
    try
      {
	verlist.resize(0);
	Int num;
	FlagVersion fv(ms_p.tableName(),"FLAG","FLAG_ROW");
	Vector<String> vlist = fv.getVersionList();
	
	num = verlist.nelements();
	verlist.resize( num + vlist.nelements() + 1, True );
	verlist[num] = String("\nMS : ") + ms_p.tableName() + String("\n");
	for(Int j=0;j<(Int)vlist.nelements();j++)
	  verlist[num+j+1] = vlist[j];
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not get Flag Version List : " << x.getMesg() << LogIO::POST;
	return False;
      }
    return True;
  }
  
  
} //#end casa namespace


// EXTRA UNUSED CODE - to try out some multi-threading on the polarization axis.

#if 0
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

if(algorithm == "tfcrop")
  {
    flagmethods_p.resize(nmethods+maxnumthreads_p,True); 
    for(Int i=nmethods; i<nmethods+maxnumthreads_p; i++)
      {
	flagmethods_p[i] = new LFTimeFreqCrop();
	flagmethods_p[i]->setParameters(parameters);
      }
  }
 else if(algorithm == "extendflags")
   {
     flagmethods_p.resize(nmethods+maxnumthreads_p,True); 
     for(Int i=nmethods; i<nmethods+maxnumthreads_p; i++)
       {
	 flagmethods_p[i] = new LFExtendFlags();
	 flagmethods_p[i]->setParameters(parameters);
       }
     //	flagmethods_p[nmethods] = new LFExtendFlags();
     //flagmethods_p[nmethods]->setParameters(parameters);
   }

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

	  {
	    AlwaysAssert( NumP == maxnumthreads_p , AipsError );
	    Int nmethods = flagmethods_p.nelements()/maxnumthreads_p;
	    Int pol=0;
	    // start pragma loop
#pragma omp parallel default(shared) private(pol)
	    {
#pragma omp for
	      for(pol=0; pol<NumP; pol++)
		{
		  runAllMethods(pol, nmethods, maxnumthreads_p,NumT, NumAnt, NumB, NumC, NumP);
		}// end of for pol/threads
	      
	    }// end pragma loop
	  }

/**************************************************************************/
/*** START UNUSED CODE *******************************************************/
/**************************************************************************/
/***********************************************/
// This function has to be thread-safe.
// For now, threadindex is on polarization.
void LightFlagger::runAllMethods(Int threadindex, Int nmethods, Int nthreads, 
				 uInt numT, uInt numAnt, uInt numB, uInt numC, uInt nPol)
{
  Cube<Float> partvisc = visc(Slice(threadindex,1) , Slice(0,numC) , Slice(0,numB*numT) );
  Cube<Bool> partflagc = flagc(Slice(threadindex,1) , Slice(0,numC) , Slice(0,numB*numT) );
  
  cout << "SHAPES : " << partvisc.shape() << partflagc.shape() << endl;
  
  for(Int i=0; i<nmethods; i++)
    {
      Int methodindex = i * (nthreads) + threadindex;
      AlwaysAssert(!flagmethods_p[methodindex].null(), AipsError);
      //os << "Running method : " << flagmethods_p[methodindex]->methodName() << LogIO::POST;
      cout << "Pol : " << threadindex << "  Running method : " << flagmethods_p[methodindex]->methodName() << endl;
      
      flagmethods_p[methodindex]->runMethod(partvisc, partflagc, numT, numAnt, numB, numC, 1);
    }
}

/**************************************************************************/
/**************************************************************************/
/***  END UNUSED CODE ************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#endif

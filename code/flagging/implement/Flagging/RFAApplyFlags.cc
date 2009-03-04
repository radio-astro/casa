
//# RFASelector.cc: this defines RFASelector
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

#include <flagging/Flagging/RFAApplyFlags.h>

#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Logging/LogIO.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <flagging/Flagging/RFASelector.h>
#include <casa/stdio.h>

#include <iomanip>

#define LOC \
  do { \
    cerr << "  at " << __FILE__ << " " << __func__ << " " << __LINE__ << endl; \
  } while (0)

namespace casa { //# NAMESPACE CASA - BEGIN

  const std::vector<FlagIndex> * RFAApplyFlags::flagIndex;

  RFAApplyFlags::RFAApplyFlags ( RFChunkStats &ch,const RecordInterface &parm )
    :
    RFAFlagCubeBase(ch,parm)
  {}

  RFAApplyFlags::~RFAApplyFlags() {}

  const RecordInterface & 
  RFAApplyFlags::getDefaults ()
  {
    static Record rec;

    // create record description on first entry
    if( !rec.nfields() )
      {
	//rec = RFAFlagCubeBase::getDefaults();
	rec.define(RF_NAME,"applyflags");
	rec.define(RF_COLUMN,"DATA");
	rec.define(RF_EXPR,"+ ABSolutely!");
	rec.define(RF_THR,.001);
	rec.define(RF_MINPOP,0);
	rec.define(RF_NBINS,50);
	rec.define(RF_PLOTCHAN,False);
	rec.define(RF_ECONOPLOT,True);
	
	rec.setComment(RF_COLUMN,"Use column: [DATA|MODEL|CORRected]");
	rec.setComment(RF_EXPR,"Expression for deriving value (e.g. \"ABS XX\", \"+ ABS XX YY\")");
	rec.setComment(RF_THR,"Probability cut-off");
	rec.setComment(RF_MINPOP,"Bin population cut-off");
	rec.setComment(RF_NBINS,"Number of bins (single value, or [NUV,NY])");
	rec.setComment(RF_PLOTCHAN,"Make plot for a specific channel, or F for no plot");
	rec.setComment(RF_ECONOPLOT,"Produce a simplified plot: T/F");
      }
    
    return rec;
  }


  void RFAApplyFlags::setIndices(const std::vector<FlagIndex> *fi) {
    LOC;

    flagIndex = fi;
  }


// This method is called after constructing the agent.
  void RFAApplyFlags::init ()
  {
    LOC;
  }

// This method is called before iterating over a chunk, to inquire the 
// expected memory use. Should return the max desired memory footprint, in MB.
// Available physical memory is divided between agents in proportion to their 
// requests.
  uInt RFAApplyFlags::estimateMemoryUse () 
  {
    LOC;
    return 1;
  }

// Called before iterating over a chunk. Returns True if agent will
// process this chunk, or False if this the agent is unable to process it.
// (this can happen if, e.g., the requisite correlations are not present).
// The Int & maxmem argument is the number of MB memory which is still 
// available in the memory pool. The agent class should plan its memory 
// use accordingly, and subtract its expected memory use from maxmem. In effect, 
// the agent "reserves" some amount of memory. This is used by RedFlagger to 
// contain the total memory footprint. Note that only a rough reckoning
// is sufficient, so only bother estimating the biggest data structures.
// See implementations in RFADiffBase and RFATimeMedian for good examples.
  Bool RFAApplyFlags::newChunk (Int &maxmem) 
  { 
    LOC; 



    //    cerr << chunk.visIter() << endl;
    //    cerr << chunk.visBuf() << endl;
    
    cerr << "MSName = " << chunk.msName() << endl;
    cerr << "Frequencies = " << chunk.frequency() << endl;
    cerr << "Antennas = " << chunk.antNames() << endl;

    cerr << "nchunck = " << chunk.nchunk() << endl;
    cerr << "npass = " << chunk.npass() << endl;
    cerr << "iTime = " << chunk.iTime () << endl;
    //    cerr << "num = " << num (StatEnums which) const
    cerr << "currentMJD = " << chunk.currentMJD () << endl;
    cerr << "startMJD = " << chunk.startMJD () << endl;
    cerr << "endMJD = " << chunk.endMJD () << endl;

    //chunk.printStats();

    cerr << (*flagIndex)[0] << endl;

#if 0
    // check correlations and figure out the active correlations mask
    Vector<Int> corrtype;
    chunk.visIter().corrType(corrtype);
    corrmask = 0;
    if( sel_corr.nelements() )
      {
	corrmask = chunk.getCorrMask(sel_corr);
	if( !corrmask )
	  {
	    if(verbose2) os<<"No matching correlations in this chunk\n"<<LogIO::POST;
	    return active=False;
	  }
      }
    else // no correlations specified so flag everything
      {
	corrmask = chunk.fullCorrMask();
      }
    
    // check field IDs and spectral window IDs
  uInt dum;
  if( sel_spwid.nelements() && !find(dum,chunk.visBuf().spectralWindow(),sel_spwid) )
  {
    if(verbose2) os<<"Spectral window does not match in this chunk\n"<<LogIO::POST;
    return active=False;
  }
  if( sel_fieldid.nelements() && !find(dum,chunk.visIter().fieldId(),sel_fieldid) )
  {
    if(verbose2) os<<"Field ID does not match in this chunk\n"<<LogIO::POST;
    return active=False;
  }
  if( sel_fieldnames.nelements() && !find(dum,chunk.visIter().fieldName(),sel_fieldnames) )
  {
    if(verbose2) os<<"Field name does not match in this chunk\n"<<LogIO::POST;
    return active=False;
  }
  if( sel_arrayid.nelements() && !find(dum,chunk.visIter().arrayId(),sel_arrayid) )
  {
    if(verbose2) os<<"Array ID does not match in this chunk\n"<<LogIO::POST;
    return active=False;
  }
  /*
  Vector<Int> temp(0);
  temp = chunk.visIter().scan(temp);
  if( temp.nelements() &&  sel_scannumber.nelements() && !find(dum,temp[0],sel_scannumber) )
  {
    os<<"Scan Number does not match the FIRST scan number in this chunk\n"<<LogIO::POST;
    return active=False;
  }
  */
  
  // Get the times at the beginning and end of this scan.
  
  const Vector<Int> &scans( chunk.visBuf().scan() );
  Int s0 = scans(0);
  if( !allEQ(scans,s0) && (quack_si >0))
    os<<"RFASelector: crash&burn, VisBuffer's given us different scans (in this chunk)."<<LogIO::EXCEPTION;
  
  //TODO : Figure out how to get the end_time and set scan_end here. Then use this in ::iterTime().
  const Vector<Double> &times( chunk.visBuf().time() );
  scan_start = times(0);
  //scan_end = times( (times.nelements())-1 );
  
  //cout << "Start time for scan " << s0 << " : " << MVTime( scan_start/C::day).string( MVTime::DMY,7)  << endl;
  

// figure out active channels (i.e. within specified segments)
  flagchan.resize();
  if( sel_freq.ncolumn() || sel_chan.ncolumn() )
  {
    flagchan = LogicalVector(num(CHAN),False);
    const Vector<Double> & fq( chunk.frequency() );
    for( uInt i=0; i<sel_freq.ncolumn(); i++ )
      flagchan = flagchan || ( fq >= sel_freq(0,i) && fq <= sel_freq(1,i) );
    Vector<Int> ch( num(CHAN) );
    indgen(ch);
    Matrix<Int> schan = sel_chan;
    schan( sel_chan<0 ) += (Int)num(CHAN);
    for( uInt i=0; i<sel_chan.ncolumn(); i++ )
    {
      flagchan = flagchan || ( ch >= schan(0,i) && ch <= schan(1,i) );
    }
    if( !sum(flagchan) )
    {
     if(verbose2)  os<<"No matching frequencies/channels in this chunk\n"<<LogIO::POST;
      return active=False;
    }
    if( allEQ(flagchan,True) )
      flagchan.resize(); // null array = all True
  }
// init all clipping mappers, and check their correlation masks
  if( sel_clip.nelements() )
  {
    // see which mappers are active for this chunk, and accumulate their
    // masks in clip_corrmask
    RFlagWord clip_corrmask=0;
    for( uInt i=0; i<sel_clip.nelements(); i++ ) 
    {
      RFlagWord mask = sel_clip[i].mapper->corrMask(chunk.visIter());
      sel_clip_active(i) = (mask!=0);
      clip_corrmask |= mask;
    }
    sum_sel_clip_active = sum(sel_clip_active);
    // If no explicit correlations were selected by the user,  
    // then use clip_corrmask as the overall mask
    if( !sel_corr.nelements() )
    {
      corrmask = clip_corrmask;
      if( !corrmask )
      {
       if(verbose2)  os<<"No matching correlations in this chunk\n"<<LogIO::POST;
        return active=False;
      }
    }
  }
// reserve a minimum of memory for ourselves
  maxmem -= 1;
// init flagging cube and off we go...
  RFAFlagCubeBase::newChunk(maxmem);
// see if full row is being flagged, i.e. no subset of channels was selected,
// and no explicit correlations (or all correlations).
  select_fullrow = (!flagchan.nelements() && 
    (!sel_corr.nelements() || corrmask==chunk.fullCorrMask()) );


#endif







    active = True;
    return active;
  };
// Called once finished with a chunk
  void RFAApplyFlags::endChunk () { LOC; }
  
// Called before starting a data pass on a chunk. 
  void RFAApplyFlags::startData () { LOC; }

// Called before starting a dry pass on a chunk. 
  void RFAApplyFlags::startDry  () { LOC; }

// Called before starting the fetch-flags pass.
  void RFAApplyFlags::startFlag () { LOC; }

// Iteration method for a flag pass. Called once per each VisBuffer.
  void RFAApplyFlags::iterFlag( uInt itime ) 
  { 
    LOC; 
    return;
  }

// Iteration methods for a data pass. Either or both may be implemented.
// iterTime() is called once for each new VisBuffer (= new time slot)
// Return value: STOP to finish iterating, CONT/DATA to continue, or DRY
// to cancel the data pass and request a dry pass.
  RFABase::IterMode RFAApplyFlags::iterTime ( uInt itime ) 
  { 
    LOC; 
    return RFABase::CONT;
  }

// iterRow() is called once per each row in the VisBuffer.
// Iterating over rows is perhaps preferrable in terms of performance,
// at least for data iterations.
  RFABase::IterMode RFAApplyFlags::iterRow  ( uInt irow ) 
  { 
    LOC;

    uInt ifr = chunk.ifrNum(irow);   // baseline for this row
    cerr << "ifr = " << ifr << endl;
    cerr << "num(CHAN) = " << num(CHAN) << endl;

    // apply data flags
    for( uInt ich=0; ich<num(CHAN); ich++ )
      //      if( !flagchan.nelements() || flagchan(ich) ) {

      //for( uInt j=0; j<sel_clip.nelements(); j++ )  {

      //Float vmin = sel_clip[j].vmin, vmax = sel_clip[j].vmax;
      //Float val = sel_clip[j].mapper->mapValue(ich,ir);
      //if( ( sel_clip[j].clip && ( val<vmin || val>vmax ) ) ||
      //(!sel_clip[j].clip && val>=vmin && val<=vmax   ) )
	
      //unflag ? flag.clearFlag(ich,ifr) : flag.setFlag(ich,ifr);
      //}

      //      flag.clearFlag(ich, ifr);

    //}
      ;

    return RFABase::CONT;
  }


// Called after a pass is completed successfully (i.e., not stopped
// by start or iter methods). Return value: STOP to stop, DATA for 
// another data pass, DRY for another dry pass.
  RFABase::IterMode RFAApplyFlags::endData   () { LOC; return RFABase::STOP; };

// Called after a dry pass is complete
  RFABase::IterMode RFAApplyFlags::endDry    () { LOC; return RFABase::STOP; };

// Called after a flag pass is complete
  void RFAApplyFlags::endFlag        () {LOC; }


} //# NAMESPACE CASA - END


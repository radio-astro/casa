//# RFAFlagExaminer.cc: this defines RFAFlagExaminer
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
#include <flagging/Flagging/RFAFlagExaminer.h>
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
#include <casa/stdio.h>
#include <map>
#include <sstream>
#include <cassert>

namespace casa { //# NAMESPACE CASA - BEGIN

  const Bool dbg3 = False;
  
  // -----------------------------------------------------------------------
  // RFAFlagExaminer constructor
  // -----------------------------------------------------------------------
  RFAFlagExaminer::RFAFlagExaminer ( RFChunkStats &ch,const RecordInterface &parm ) : 
    RFASelector(ch, parm)//,RFDataMapper(parm.asArrayString(RF_EXPR),parm.asString(RF_COLUMN))
  {
    if(dbg3)  cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;
    //desc_str = String("flagexaminer");
    if(dbg3) cout<<"FlagExaminer constructor "<<endl;

    totalflags    = accumTotalFlags    = 0;
    totalcount    = accumTotalCount    = 0;
    totalrowflags = accumTotalRowFlags = 0;
    totalrowcount = accumTotalRowCount = 0;
    //parseParm(parm);

    os = LogIO(LogOrigin("RFAFlagExaminer", "RFAFlagExaminer", WHERE));

    accumflags.clear();
    accumtotal.clear();


    // Handle in-row selections, the following is a
    // copy-paste from RFASelector2

    char s[256];
    // parse input arguments: channels
    if( parseRange(sel_chan,parm,RF_CHANS)) 
      {
        String sch;
        for( uInt i=0; i<sel_chan.ncolumn(); i++) 
          {
            sprintf(s,"%d:%d",sel_chan(0,i),sel_chan(1,i));
            addString(sch,s,",");
          }
        addString(desc_str, String(RF_CHANS) + "=" +sch);
        sel_chan(sel_chan>=0) += -(Int)indexingBase();
      }

    // parse input arguments: correlations
    if( fieldType(parm,RF_CORR,TpString,TpArrayString))
      {
        String ss;
        Vector<String> scorr( parm.asArrayString(RF_CORR)) ;
        sel_corr.resize( scorr.nelements()) ;
        for( uInt i=0; i<scorr.nelements(); i++) 
          {
            sel_corr(i) = Stokes::type( scorr(i)) ;
            if( sel_corr(i) == Stokes::Undefined) 
              os<<"Illegal correlation "<<scorr(i)<<endl<<LogIO::EXCEPTION;
            addString(ss,scorr(i),",");
          }
        addString(desc_str,String(RF_CORR)+"="+ss);
      }
  }
  
 
  RFAFlagExaminer::~RFAFlagExaminer()
  {
    if(dbg3)  cout << "FlagExaminer destructor " << endl;    
  }

  Bool RFAFlagExaminer::newChunk(Int &maxmem)
    {
      /* For efficiency reasons, use arrays to collect 
         histogram data for in-row selections
      */
      accumflags_channel = vector<uInt64>(chunk.num(CHAN), 0);
      accumtotal_channel = vector<uInt64>(chunk.num(CHAN), 0);
      accumflags_correlation = vector<uInt64>(chunk.num(CORR), 0);
      accumtotal_correlation = vector<uInt64>(chunk.num(CORR), 0);
          
      return RFASelector::newChunk(maxmem);
    }

  
  void RFAFlagExaminer::initialize()
  {
    if(dbg3)  cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;

    totalflags    = 0;
    totalcount    = 0;
    totalrowflags = 0;
    totalrowcount = 0;
    inTotalFlags =
	inTotalCount = 
	inTotalRowCount = 
	outTotalFlags =
	outTotalCount = 
	outTotalRowCount = 
	outTotalRowFlags = 0;
  }

  // Is not called if this is the only agent
  void RFAFlagExaminer::finalize()
  {
    //cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;

    return;
  }
  // -----------------------------------------------------------------------
  // processRow
  // Raises/clears flags for a single row, depending on current selection
  // -----------------------------------------------------------------------
  void RFAFlagExaminer::processRow(uInt ifr, uInt it)
  {
      // called often... if(dbg3)  cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;      
      return;
  }
  
  void RFAFlagExaminer::startFlag (bool verbose)
  {
    if(dbg3)  cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;

    totalflags    = 0;
    totalcount    = 0;
    totalrowflags = 0;
    totalrowcount = 0;
    
    inTotalFlags = 
	inTotalCount = 
	inTotalRowCount =
	outTotalFlags = 
	outTotalCount =
	outTotalRowCount = 
	outTotalRowFlags = 0;

    RFAFlagCubeBase::startFlag(verbose);

    return;
  }
  
  void RFAFlagExaminer::initializeIter (uInt it) 
  {
      if(dbg3)  cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;

      for(unsigned ii=0;
	  ii<chunk.visBuf().flagRow().nelements();
	  ii++)
	  if (chunk.visBuf().flagRow()(ii)) {
	      inTotalRowFlags++;
	  }
      
      inTotalRowCount += chunk.visBuf().flagRow().nelements();
      
      for(Int ii=0;
	  ii<chunk.visBuf().flag().shape()(0);
	  ii++)
	  for(Int jj=0;
	      jj<chunk.visBuf().flag().shape()(1);
	      jj++)
	      if (chunk.visBuf().flag()(ii,jj)) inTotalFlags++;
  }

  // Is not called if this is the only agent
  void RFAFlagExaminer::finalizeIter (uInt it) 
  {
    //cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;

    outTotalRowCount += chunk.visBuf().flagRow().nelements();

    for (unsigned ii = 0;
	 ii < chunk.visBuf().flagRow().nelements();
	 ii++)
	if (chunk.visBuf().flagRow()(ii)) {
	    outTotalRowFlags++;
	}

    for (Int ii=0;
	 ii<chunk.visBuf().flag().shape()(0);
	 ii++) {

	outTotalCount += chunk.visBuf().flag().shape()(1);
	
	for (Int jj=0;
	     jj<chunk.visBuf().flag().shape()(1);
	     jj++) {
	    if (chunk.visBuf().flag()(ii,jj)) outTotalFlags++;
	}
    }
  }

  // it: time index
  void RFAFlagExaminer::iterFlag(uInt it)
  {
    if(dbg3)  cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;

    // Set the flags and count them up.
    RFASelector::iterFlag(it);
    
    // count if within specific timeslots
    const Vector<Double> &times( chunk.visBuf().time() );
    Double t0 = times(it);
    
    Bool within_time_slot = True;
    
    if (sel_time.ncolumn()) {

	if( anyEQ(sel_timerng.row(0) <= t0 && 
		  sel_timerng.row(1) >= t0, True) )
	    within_time_slot = True;
	else within_time_slot = False;
    }
    
    if (within_time_slot) {

	// More counting and fill up final display variables.
	const Vector<Int> &ifrs( chunk.ifrNums() );
	const Vector<Int> &feeds( chunk.feedNums() );
	const Vector<casa::RigidVector<casa::Double, 3> >&uvw( chunk.visBuf().uvw() );

        unsigned spw = chunk.visBuf().spectralWindow();
        unsigned field = chunk.visBuf().fieldId();
        const Vector<Int> &antenna1( chunk.visBuf().antenna1() );
        const Vector<Int> &antenna2( chunk.visBuf().antenna2() );
        const Vector<Int> &scan    ( chunk.visBuf().scan() );

        const Vector<String> &antenna_names( chunk.antNames()) ;

	// Vector<Vector<Double> > &uvw=NULL;//( chunk.visIter.uvw(uvw) );
	//chunk.visIter().uvw(uvw);
	Double uvdist=0.0;
	
	// loop over rows
	for (uInt i=0; i < ifrs.nelements(); i++) {
	    Bool inrange=False;
	    
	    uvdist = sqrt( uvw(i)(0)*uvw(i)(0) + uvw(i)(1)*uvw(i)(1) );
	    
	    for( uInt j=0; j<sel_uvrange.ncolumn(); j++) {
		if( uvdist >= sel_uvrange(0, j) &&
		    uvdist <= sel_uvrange(1, j) ) 
		    
		    inrange |= True;
	    }

	    if( (!sel_ifr.nelements() || sel_ifr(ifrs(i))) && 
		(!sel_feed.nelements() || sel_feed(feeds(i))) &&
		(!sel_uvrange.nelements() || inrange ) )
	      {
		// Operate on the chosen row.
		// Collect counts.
		
		//cout << "selected row for " << ifrs(i) << "," << it << endl;
		
                if(chunk.nfIfrTime(ifrs(i),it) == chunk.num(CORR)*chunk.num(CHAN))
                    totalrowflags++;

                totalrowcount++;

                uInt64 f = chunk.nfIfrTime(ifrs(i), it);
                uInt64 c = chunk.num(CORR) * chunk.num(CHAN);

                // need nfIfrTimeCorr
                // need nfIfrTimeChan

                totalflags += f;
                totalcount += c;

                /* Update histograms */
                
                /* histogram baseline */
                {
                  string baseline = 
                    antenna_names(antenna1(i)) + "&&" +
                    antenna_names(antenna2(i));
                  
                  accumflags["baseline"][baseline] += f;
                  accumtotal["baseline"][baseline] += c;
                }
                
                /* histogram antenna */
                {
                  /* Careful here, update the counts for both
                     antenna1 and antenna2, unless they are the same.
                  */
                  accumflags["antenna"][antenna_names(antenna1(i))] += f;
                  accumtotal["antenna"][antenna_names(antenna1(i))] += c;
                  if (antenna1(i) != antenna2(i)) {
                    accumflags["antenna"][antenna_names(antenna2(i))] += f;
                    accumtotal["antenna"][antenna_names(antenna2(i))] += c;
                  }
                }

                /* histogram spw */
                {
                  stringstream spw_string;
                  spw_string << spw;
                  accumflags["spw"][spw_string.str()] += f;
                  accumtotal["spw"][spw_string.str()] += c;
                }

                /* histogram fieldID */
                {
                  stringstream fieldID_string;
                  fieldID_string << field;
                  accumflags["field"][fieldID_string.str()] += f;
                  accumtotal["field"][fieldID_string.str()] += c;
                }
                /* histogram scan */
                {
                  stringstream scan_string;
                  scan_string << scan(i);
                  accumflags["scan"][scan_string.str()] += f;
                  accumtotal["scan"][scan_string.str()] += c;
                }
              }
        }
    }
    
    return;
  }

  /* Update histogram for channel and correlation
     This cannot happen in iterFlag(), which is called once per chunk,
     because the "flag" cursor needs to be updated once per row.
  */
  RFA::IterMode RFAFlagExaminer::iterRow(uInt irow)
    {
      unsigned ifr = chunk.ifrNum(irow);
      
      for( uInt ich = 0;
           ich < chunk.num(CHAN); 
           ich++ ) {

        if( !flagchan.nelements() || flagchan(ich) ) {
          
          RFlagWord corrs = flag.getFlag(ich, ifr);
          unsigned n_flags = 0;

          for (uInt icorr = 0; icorr < chunk.num(CORR); icorr++) {
              if (corrs & 1) {
                  accumflags_correlation[icorr] += 1;
                  n_flags += 1;
              }
              accumtotal_correlation[icorr] += 1;
              corrs >>= 1;
          }

          accumflags_channel[ich] += n_flags;
          accumtotal_channel[ich] += chunk.num(CORR);
        }
      }
      
      return RFA::CONT;
    }

  void RFAFlagExaminer::endChunk ()
    {
        RFASelector::endChunk();

        for (unsigned ich = 0; ich < chunk.num(CHAN); ich++) {
            if (accumtotal_channel[ich] > 0) {
                stringstream ss;
                ss << ich;
                accumflags["channel"][ss.str()] = accumflags_channel[ich];
                accumtotal["channel"][ss.str()] = accumtotal_channel[ich];
            }
        }

        for (unsigned icorr = 0; icorr < chunk.num(CORR); icorr++) {
            if (accumtotal_correlation[icorr] > 0) {
                stringstream ss;
                ss << icorr;
                accumflags["correlation"][ss.str()] = accumflags_correlation[icorr];
                accumtotal["correlation"][ss.str()] = accumtotal_correlation[icorr];
            }
        }
    }


  void RFAFlagExaminer::endFlag ()
  {
    if(dbg3)  cout << __FILE__ << ":" << __func__ << "():" << __LINE__ << endl;
        
    char s[1024];
    sprintf(s,"Chunk %d (field %s, fieldID %d, spw %d)",
     	    chunk.nchunk(),
            chunk.visIter().fieldName().chars(),
            chunk.visIter().fieldId(),
            chunk.visIter().spectralWindow());
    os << "---------------------------------------------------------------------" << LogIO::POST;
    os<<s<<LogIO::POST;
    
    sprintf(s, "%s, %d channel%s, %d time slots, %d baselines, %d rows\n",
     	    chunk.getCorrString().chars(),
	    chunk.num(CHAN),
	    chunk.num(CHAN) == 1 ? "" : "s",
	    chunk.num(TIME),
     	    chunk.num(IFR),
	    chunk.num(ROW));
    os << s << LogIO::POST;
    
    os << "\n\n\nData Selection to examine : " << desc_str ;
    if(flag_everything) os << " all " ;
    os << LogIO::POST;
    
    Double ffrac=0.0,rffrac=0.0;
    if(totalcount) ffrac = totalflags*100.0/totalcount;
    if(totalrowcount) rffrac = totalrowflags*100.0/totalrowcount;

    // LogSink cannot handle uInt64...
    std::stringstream ss;
    ss << totalrowflags << " out of " << totalrowcount <<
	" (" << rffrac << "%) rows are flagged.";
    os << ss.str() << LogIO::POST;

    ss.str("");

    ss << totalflags << " out of " << totalcount <<
	" (" << ffrac << "%) data points are flagged.\n\n";

    os << ss.str() << LogIO::POST;
    
    os << "---------------------------------------------------------------------" << LogIO::POST;
    
    accumTotalFlags    += totalflags;
    accumTotalCount    += totalcount;
    accumTotalRowFlags += totalrowflags;
    accumTotalRowCount += totalrowcount;
    
    return;
  }

  /*
    Return results of this run over all chunks:
    How many flags were set / unset
   */
  Record RFAFlagExaminer::getResult()
    {
      Record r;

      r.define("flagged", (uInt) accumTotalFlags);
      r.define("total"  , (uInt) accumTotalCount);


      for (map<string, map<string, uInt64> >::iterator j = accumtotal.begin();
           j != accumtotal.end();
           j++) {
        /* Note here: loop over the keys of accumtotal, not accumflags,
           because accumflags may not have all channel keys */
        
          Record prop;
          for (map<string, uInt64>::const_iterator i = j->second.begin();
               i != j->second.end();
               i++) {
            
              Record t;

              t.define("flagged", (uInt) accumflags[j->first][i->first]);
              t.define("total", (uInt) i->second);
              
              prop.defineRecord(i->first, t);
          }
          
          r.defineRecord(j->first, prop);
      }
      
      return r;
    }

  
} //# NAMESPACE CASA - END


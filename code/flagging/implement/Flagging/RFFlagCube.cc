
//# RFFlagCube.cc: this defines RFFlagCube
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
#include <flagging/Flagging/RFFlagCube.h>
#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/LogiVector.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/Time.h>
#include <casa/Quanta/MVTime.h>
#include <stdio.h>

#include <casa/System/PGPlotterInterface.h>
        
namespace casa { //# NAMESPACE CASA - BEGIN

const Bool dbg=False;
const Bool mdbg=False;
const Bool verbose=False;
        
RFCubeLattice<RFlagWord> RFFlagCube::flag; // global flag lattice
FlagMatrix RFFlagCube::flagrow;   
Int RFFlagCube::pos_get_flag=-1,RFFlagCube::pos_set_flag=-1;
Int RFFlagCube::maxmemuse=0;

RFlagWord RFFlagCube::base_flagmask=1,
          RFFlagCube::full_corrmask;
Int RFFlagCube::agent_count=0,RFFlagCube::num_inst=0;
RFlagWord RFFlagCube::agent_corrmasks[sizeof(RFlagWord)*8];
Vector<RFlagWord> RFFlagCube::corr_flagmask;  
//This is a map from a set of correlations to a set of agents, i.e.
// which agents deal with any of the given correlations

Bool RFFlagCube::reset_preflags;
  LogIO RFFlagCube::default_sink(LogOrigin("Flagger","FlagCube"));

PGPlotterInterface * RFFlagCube::report_plotted=NULL;
 String RFFlagCube::agent_names("");

RFFlagCube::RFFlagCube ( RFChunkStats &ch,Bool ignore,Bool reset,LogIO &sink )
  : chunk(ch),os(sink)
{
  num_inst++;
  if( reset )
  {
    pfpolicy = FL_RESET;
    if(verbose) os<<"Existing flags will be reset\n"<<LogIO::POST;
  }
  else
  {
    if( ignore )
    {
      pfpolicy = FL_IGNORE;
      if(verbose) os<<"Existing flags will be ignored, but added to\n"<<LogIO::POST;
    }
    else
    {
      pfpolicy = FL_HONOR;
      if(verbose) os<<"Existing flags will be honored\n"<<LogIO::POST;
    }
  }
}

RFFlagCube::~RFFlagCube ()
{
  num_inst--;
}

uInt RFFlagCube::estimateMemoryUse ( const RFChunkStats &ch )
{
    return
        ch.num(CHAN) * ch.num(IFR) * ch.num(TIME) * 
        sizeof(RFlagWord) / (1024*1024) + 1;
    
}

// creates flag cube for a given visibility chunk
void RFFlagCube::init( RFlagWord corrmsk, uInt nAgent, bool is_selector, const String &name) 
{
    kiss = (nAgent == 1 && is_selector); /* Use a Cube<Bool> instead of the flag lattice in this case */

    if (dbg) cout << "name=" << name << endl;
 
    // setup some masks
    corrmask = corrmsk;
    check_corrmask = (pfpolicy == FL_HONOR) ? corrmsk : 0;
    check_rowmask  = (pfpolicy == FL_HONOR) ? RowFlagged : 0;
 
    // clear stats  
    tot_fl_raised=tot_row_fl_raised=fl_raised=fl_cleared=
	row_fl_raised=row_fl_cleared=0;
    
    // init flag cube if it is empty
    if ( !flag.shape().nelements() ) {
    
	reset_preflags=False;

	// setup correlation masks. The first NCORR bits of the flag word
	// are used to store the apriori flags. Basemask is the first bitmask
	// actually used for flagging
	base_flagmask = num(CORR)>=2 ? 1<<num(CORR) : 4;  // is this a bug, should base_flagmask be 2 if num(CORR) is 1?

	// full_corrmask is the mask of all correlations flagged
	full_corrmask = (1<<num(CORR))-1;

        if (!kiss) {
            // init empty flag lattice
            // initial state is all pre-flags set; we'll clear them as we go along
            flag.init(num(CHAN),num(IFR),num(TIME),num(CORR), nAgent, full_corrmask, maxmemuse, 2);
        }
        else {
            /* Set shape to a dummy value, 
               only so that we can later use flag.shape().nelements()
               in the if conditions.
            */
            flag.shape().resize(1);
        }
	pos_get_flag = pos_set_flag = -1;

	// allocate cube of row flags
	flagrow.resize(num(IFR),num(TIME));
	flagrow = RowFlagged|RowAbsent; // 0000 0011

	// reset instance counters 
	agent_count = 0; // reset instantiation counts
	corr_flagmask.resize(1<<num(CORR));
	corr_flagmask = 0;
	agent_names = "";
	report_plotted = NULL;
    }
    //cout << "uint_max=" << UINT_MAX << endl;
    flagmask = base_flagmask<<agent_count;
    if (dbg) cout << "agent_count=" << agent_count 
		  << " base_flagmask=" << base_flagmask 
		  << " flagmask=" << (flagmask > UINT_MAX) << endl;
    //exit(0);
    if( !flagmask  )
	throw(AipsError("Too many flagging agents instantiated"));
    agent_corrmasks[agent_count] = corrmask;
    agent_count++;
    // raise flag if any one instance has a RESET pre-flag policy
    if ( pfpolicy==FL_RESET )
	reset_preflags=True;

    // set bits in corr_flagmask
    for ( uInt cm=0; cm < corr_flagmask.nelements(); cm++ )
	if ( cm & corrmask )
	    corr_flagmask(cm)|=flagmask;
  
    // accumulates names of all agents using our cube. (This is just eye 
    // candy, for plots)
    if ( name.length() )
	{
	    // strip off instance count from name, if it's there
	    String nm(name);
	    int pos = nm.index(Regex("#[0-9]+$"));
	    if ( pos>0 )
		nm = nm.before(pos);
	    // add to list of names
	    if ( !agent_names.contains(nm) )
		{
		    if ( agent_names.length() )
			agent_names += "/";
		    agent_names += nm;
		}
	}
    if(dbg) cout << "End of init. reset_preflags : " << reset_preflags << endl;
}

// deallocates flag cube
void RFFlagCube::cleanup ()
{
    if (flag.shape().nelements()) {
        flag.cleanup();
        flagrow.resize(0,0);
        corr_flagmask.resize(0);
        agent_count=0;
    }
}

void RFFlagCube::reset ()
{
    fl_raised=fl_cleared=row_fl_raised=row_fl_cleared=0;
    my_corrflagmask = corr_flagmask(corrmask);
    if (!kiss) flag.reset();

    return;
}

String RFFlagCube::getSummary ()
{
  char s[128];
  sprintf(s, "%d pixel flags, %d row flags",
	  tot_fl_raised,tot_row_fl_raised);
  return s;
}

// prints flagging statistics
void RFFlagCube::printStats ()
{
  if( tot_fl_raised )
    dprintf(os,"%d pixel flags; %d raised this pass, %d cleared\n",
        tot_fl_raised,fl_raised,fl_cleared);
  if( tot_row_fl_raised )
    dprintf(os,"%d row flags; %d raised this pass, %d cleared\n",
        tot_row_fl_raised,row_fl_raised,row_fl_cleared);
}

// Sets flag at (ich,iifr) to 1. Returns True if flag has not been raised
// previously.
Bool RFFlagCube::setFlag ( uInt ich,uInt ifr, FlagCubeIterator &iter )
{
    if (dbg) cerr << "flag for " << ich << "," << ifr << "corrmask = " << corrmask;

    if (kiss) {
        uInt c = 1;
        bool raised = false;
        for (uInt icorr = 0; icorr < num(CORR); icorr++, c<<=1) {
            if (c & corrmask) {
                if (!in_flags(icorr, ich, ifr)) {
                    raised = true;
                }
                in_flags(icorr, ich, ifr) = 1;
            }
        }
        if (raised) {
            tot_fl_raised++;
            fl_raised++;
        }
        return raised;
    }

    RFlagWord oldfl = iter(ich,ifr);
    if (dbg) cerr << " : " << oldfl << "," << flagmask;
    if ( !(oldfl&flagmask) ) {
	tot_fl_raised++;
	fl_raised++;
	if (dbg) cerr << " setting " << oldfl << " | " << flagmask << endl;
	iter.set(ich, ifr, oldfl | flagmask);
	if (dbg) cerr << " -----> true --> " << iter(ich,ifr) << endl;
	return True;
    }
    if (dbg) cerr << " -----> false --> " << iter(ich,ifr) << endl;
    return False;
}

// Clears flag at (ich,iifr). Returns True if flag was up before.
Bool RFFlagCube::clearFlag ( uInt ich,uInt ifr,FlagCubeIterator &iter )
{
    if(dbg) cerr << "unflag for " << ich << "," << ifr;

    if (kiss) {
        
        uInt c = 1;
        bool cleared = false;
        for (uInt icorr = 0; icorr < num(CORR); icorr++, c<<=1) {
            if (c & corrmask) {
                if (in_flags(icorr, ich, ifr)) {
                    cleared = true;
                    in_flags(icorr, ich, ifr) = 0;
                }
            }
        }
        if (cleared) {
            tot_fl_raised--;
            fl_cleared++;
        }
        return cleared;
    }

    RFlagWord oldfl = iter(ich,ifr);
    if(dbg)cerr << " : " << oldfl << "," << flagmask;

    // all flags cleared for this point - update global stats
    
    if( !(oldfl&flagmask) ) {
        tot_fl_raised--;
        fl_cleared++;
        iter.set(ich, ifr, oldfl & flagmask);
        if(dbg)cerr << " -----> true --> " << iter(ich,ifr) << endl;
        return True;
    }
    if(dbg)cerr << " -----> false --> " << iter(ich,ifr) << endl;
    return False;
}

// Sets flag at (ifr,itime). Returns True if flag has not been raised
// previously.
Bool RFFlagCube::setRowFlag ( uInt ifr, uInt itime )
{
    RFlagWord oldfl = flagrow(ifr,itime);

    // first flag raised for this row - update global stats
    if( !(oldfl&flagmask) )
        {
            tot_row_fl_raised++;
            row_fl_raised++;
            flagrow(ifr,itime) = oldfl | flagmask;
            return True;
        }
    return False;
}

// Clears row flag for (iifr,it). Returns True if flag was up before.
Bool RFFlagCube::clearRowFlag ( uInt ifr,uInt itime )
{
    RFlagWord oldfl = flagrow(ifr,itime);

    // all flags cleared for this point - update global stats
    if( !(oldfl&flagmask) )  {
        tot_row_fl_raised--;
        row_fl_cleared++;
        flagrow(ifr,itime) = oldfl & flagmask;
        return True;
    }
    return False;
}

// Advances the global flag lattice iterator to the specified time.
// If pfr and pfc are specified, fills in data
void RFFlagCube::advance( uInt it,Bool getFlags )
{
  if( !kiss && flag.position() != (Int)it )
    flag.advance(it);
  if( getFlags )
    getMSFlags(it);
  return;
}

// Fills lattice with apriori flags (from VisBuffer in ChunkStats)
void RFFlagCube::getMSFlags(uInt it)
{
  // return if already filled at this iterator position
  if( !kiss ) {
      if (flag.position() <= pos_get_flag )
          return;
      
      pos_get_flag = flag.position();
  }
  else {
      pos_get_flag = it;
  }
  
  FlagVector fl_row (flagrow.column(pos_get_flag));

  const Vector<Bool> & fr( chunk.visBuf().flagRow() );

  in_flags.resize(num(CORR), num(CHAN), num(IFR));
    
  if( reset_preflags ) // RESET pre-flag policy: reset the flags
  {
    for( uInt i=0; i<fr.nelements(); i++ )
    {
      uInt ifr = chunk.ifrNum(i);

      if (!kiss) {
          // clear row flag
          fl_row(ifr) &= ~(RowAbsent|RowFlagged); // 0000 0011 & 1111 1100 = 0000 0000
          // clear pixel flags
          flag.set_column(ifr, 0); // 0000 0000
      }

      for (uInt ichan = 0; ichan < num(CHAN); ichan++) {
          for (uInt icorr = 0; icorr < num(CORR); icorr++) {
              in_flags(icorr, ichan, ifr) = 0;
          }
      }
    }
  }
  else // HONOR/IGNORE policy: faithfully copy flags from FLAG and FLAG_ROW
  {
    const Cube<Bool>   & fc( chunk.visBuf().flagCube() );

    if (fc.shape()(0) != (Int) num(CORR) ||
	fc.shape()(1) != (Int) num(CHAN))
      {
	stringstream ss;
	ss << "The shape of FLAGs has changed (from (" << num(CORR) << ", " << num(CHAN)
	   << ") to (" << fc.shape()(0) << ", " << fc.shape()(1) << ")) within the same chunk. "
	   << "Invalid MS.";
	throw AipsError(ss.str());
      }

    Bool deleteIn, deleteFc;
    Bool *inp = in_flags.getStorage(deleteIn);
    const Bool *fcp = fc.getStorage(deleteFc);

    for( uInt i=0; i < fr.nelements(); i++ )
    {
      uInt ifr = chunk.ifrNum(i);

      if (fr(i)) {
          unsigned n = num(CHAN)*num(CORR);
          for (unsigned j = 0; j < n; j++) {
              inp[j + ifr * n] = 1; 
          }
      }
      else {
          unsigned n = num(CORR) * num(CHAN);
          for (unsigned j = 0; j < n; j++) {
              inp[j + n * ifr] = fcp[j + n * i];
          }
      }

      if (!kiss) {
          fl_row(ifr) &= ~RowAbsent; // 0000 0011 & 11111101 = 0000 0001
          // initial state of lattice is all correlations flagged, so we just
          // ignore flagged rows
          //if( !fr(i) )  // row not flagged, or we ignore/reset flags
          //{
          // clear row flag in internal matrix, if needed
          if( !fr(i) ) 
              fl_row(ifr) &= ~RowFlagged; // 0000 0001 & 1111 1110 -> 0000 0000
          /* clear all row flags...so that only new flags are True at the end */
          
          ///... read in chan flags for all rows......
          ///...  because all may need to be written back.
          
          
          /* The lattice was initialized to all flags set,
             Now clear as appropriate (if not FLAG_ROW and not FLAG)
          */
          if (num(CORR) == 1) {
              for (uInt ich=0; ich<num(CHAN); ich++ ) {
                  if( !fl_row(ifr) && !fc(0, ich, i) ) {
                      flag.set(ich, ifr, 0, 0);
                  }
              }
          }
          else {
              for (uInt ich=0; ich<num(CHAN); ich++ ) {
                  for (uInt icorr=0; icorr<num(CORR); icorr++ ) {
                      
                      if( !fl_row(ifr) && !fc(icorr, ich, i) ) {
                          //(*flag.cursor())(ich,ifr) &= ~(1<<icorr); 
                          
                          flag.set(ich, ifr, icorr, 0);
                      }
                  }
              }
          }
      }
    }
    in_flags.putStorage(inp, deleteIn);
    fc.freeStorage(fcp, deleteFc);
  }
}


// Moves flags from lattice to VisBuffer
// ifrnums is a vector of IFR indices (derived from antenna indices)
void RFFlagCube::setMSFlags(uInt itime)
{
    //    cerr << itime << " write to visIter" << endl;
  //jmlarsen: This function should probably use the flagrow member variable.

    if(mdbg) 
    {
      cerr << "RFFlagCube :: setMSFlags for " ;
      cerr << "itime : " << itime << endl;
    }

  // return if already done at this iterator position
  if (!kiss) {
      if( flag.position() <= pos_set_flag )
          return;
      
      pos_set_flag = flag.position();
  }

  uInt nr = chunk.visBuf().nRow();
  Vector<Bool> out_flagrow( nr,False );
  Cube<Bool>   out_flagcube( num(CORR),num(CHAN),nr,False );

  chunk.nrfTime(itime) = 0;

  Bool deleteOut, deleteIn;
  Bool *outp = out_flagcube.getStorage(deleteOut);
  const Bool *inp = in_flags.getStorage(deleteIn);

  Bool deleteNfChanIfr;
  uInt *nfChanIfrp = chunk.nfChanIfr().getStorage(deleteNfChanIfr);

  unsigned ncorr = num(CORR);
  unsigned nchan = num(CHAN);

  for( uInt ir=0; ir<nr; ir++ )
  {
      uInt ifr = chunk.ifrNum(ir);

      chunk.nrfIfr(ifr) = 0;

      if (dbg) cerr << "  at " << __FILE__ << " " << __func__ << " " << __LINE__ << " " << __LINE__ << out_flagrow(ir) << endl;
      
      // Set data flags
      unsigned n = nchan * ncorr;
      unsigned iout = n*ir;
      unsigned iin = n*ifr;
      unsigned iChanIfr = nchan * ifr;
      uInt &iNfIfrTime = chunk.nfIfrTime(ifr, itime);
      iNfIfrTime = 0;
      for( uInt ich=0; ich < nchan; ich++, iChanIfr++) {
          nfChanIfrp[iChanIfr] = 0;
      }

      iChanIfr = nchan * ifr;
      for( uInt ich=0; ich < nchan; ich++, iChanIfr++) {

          if (kiss) {

              if (ncorr == 1) {
                  if (outp[iout++] = inp[iin++]) {
                      nfChanIfrp[iChanIfr]++;
                      iNfIfrTime++;
                  }
              }
              else {
                  for( uInt icorr = 0; icorr < ncorr; icorr++, iout++, iin++) {
                      if (outp[iout] = inp[iin]) {
                          nfChanIfrp[iChanIfr]++;
                          iNfIfrTime++;
                      }
                  }
              }
          } else {

              RFlagWord fw = flag(ich, ifr);
              
              if (fw) {
                  // if anything was raised for this channel
                  
                  // loop over correlations and see which are (a) preflagged
                  // (b) been flagged by agents. 
                  RFlagWord cmask = 1;
                  if (num(CORR) == 1) {
                      
                      if (fw & 1 || fw & corr_flagmask(1)) {
                          out_flagcube(0, ich, ir) = True;
                          chunk.nfChanIfr(ich,ifr)++;
                          chunk.nfIfrTime(ifr,itime)++;
                      }
                  }
                  else {
                      for( uInt  icorr=0; icorr<num(CORR); icorr++, cmask<<=1 ) {
                          
                          if( fw & cmask       // (a) if fw is set for this correlation
                              ||
                              fw & corr_flagmask(cmask) // (b) if agent flag in fw
                              // is set for any agent that 
                              // deals with this correlation
                              ) {
                              
                              out_flagcube(icorr,ich,ir) = True;
                              chunk.nfChanIfr(ich,ifr)++;
                              chunk.nfIfrTime(ifr,itime)++;
                          }
                      }
                  }
              }
          }
      }
    
      /* if any dataflags have been unflagged, they already are.
	 if any rowflags have been unflagged, this is already in the dataflags too */
      /* if any dataflags have been flagged - this info is there in dataflags.
	 if any rowflags have been flagged, this is also there in dataflags */
      // so make flag_row the AND of the dataflags.

      /* Set flagrow if everything was flagged */

      /* Fill in all the flag counts here */
      // chunk.nf*
      // nrfIfr(ifr), nrfTime(itime), nfIfrTime(ifr,itime), nfChanIfr(ich,ifr)

      if (chunk.nfIfrTime(ifr, itime) == nchan * ncorr) {

          out_flagrow(ir) = True;

          chunk.nrfIfr(ifr)++;
          chunk.nrfTime(itime)++;
      }
  }

  out_flagcube.putStorage(outp, deleteOut);
  in_flags.freeStorage(inp, deleteIn);
  //  chunk.nfChanIfr().putStorage(nfChanIfrp, deleteNfChanIfr);

  if(mdbg)
      {
          Int cnt1=0,cnt2=0;
          for( uInt ir=0; ir<nr; ir++ )
	      {
                  uInt ifr = chunk.ifrNum(ir);
                  cnt1 += chunk.nrfIfr(ifr);
                  cnt2 += chunk.nfIfrTime(ifr,itime);
                  cerr << "Sum of flagrow (ifr) : " << cnt1 << endl;
                  cerr << "Sum of flags (ifr,itime): " << cnt2 << endl;
	      }
      }

  chunk.visIter().setFlag(out_flagcube);
  chunk.visIter().setFlagRow(out_flagrow);
}

const Float tr_trivial_array[] = {-1,1,0,-1,0,1}; 
const Vector<Float> tr_trivial(IPosition(1,6),tr_trivial_array);

void RFFlagCube::plotImage ( PGPlotterInterface &pgp,const Matrix<Float> &img,
    const char *lx,const char *ly,const char *ltop,
    Bool wedge,Float xbox,Float ybox,Bool xfreq )
{
// setup plot 
  pgp.env(-.5,img.nrow()-.5,-.5,img.ncolumn()-.5,0,-2);
  pgp.lab(lx,ly,ltop);
  pgp.gray(img,max(img),0,tr_trivial);
// select box options based on plot type
  String xopt( xbox!=0 ? "BC" : ( xfreq ? "BCSTI" : "BCNSTI" )),
         yopt( ybox!=0 ? "BC" : "BCNSTIV");
  Int xtick = 0;
  if( xfreq ) // manually choose X tick interval for a frequency axis
    xtick = (Int)pgp.rnd(img.nrow()/6.,5);
// draw box and color bar 
  pgp.box(xopt,xtick,0,yopt,0,0);
  if( wedge )
  {
    pgp.wedg("RI",.5,4,max(img),0,"");
    if( max(img)>100.01 )  // .01 avoids precision effects
    { 
      IPosition imin(2),imax(2);
      Float vmin,vmax;
      minMax(vmin,vmax,imin,imax,img);
      default_sink<<LogIO::WARN<<"Oops, flag density "<<vmax<<"% in plot \""<<ltop<<"\"\n"
          "at position "<<imax(0)<<","<<imax(1)<<"\n"
          "Please submit a bug report!\n"<<LogIO::POST;
    }
  }
// label frequencies
  if( xfreq ) 
  {
    uInt nchan = img.nrow();
    const Vector<Double> & fq( chunk.frequency() );
    for( uInt ich=0; ich<nchan; ich+=xtick )
    {
      char s[64];
      sprintf(s,"%d(%0.2f)",ich,fq(ich)*1e-6);
      pgp.mtxt("B",1.5,(ich+.5)/nchan,.5,s);
    }
  }
// draw grid along X axis  
  if( xbox!=0 )
  {
    uInt nx =(uInt)(img.nrow()/xbox+.5);
    Vector<Float> x(2),y(2);
    y(0)=-.5; y(1)=img.ncolumn()-.5;
    for( uInt i=1; i<nx; i++ )
    {
      x=i*xbox-.5;
      pgp.line(x,y);
    }
  }
// draw grid along Y axis  
  if( ybox!=0 )
  {
    uInt ny =(uInt)(img.ncolumn()/ybox+.5);
    Vector<Float> x(2),y(2);
    x(0)=-.5; x(1)=img.nrow()-.5;
    for( uInt i=1; i<ny; i++ )
    {
      y=i*ybox-.5;
      pgp.line(x,y);
    }
  }
// // draw max marks per rows
//   Vector<Float> rowsum(img.nrow());
//   for( uInt i=0; i<img.nrow(); i++ )
//     rowsum(i) = sum( img.row(i) );
//   Float vmin,vmax; IPosition dum(1);
//   minMax(vmin,vmax,dum,dum,rowsum);
//   LogicalVector wh( rowsum > vmax-(vmax-vmin)*.5f );
//   for( uInt i=0; i<img.nrow(); i++ )
//     if( wh(i) )
//       pgp.arro(i,(img.ncolumn()-.5)*1.02,i,img.ncolumn()-.5);
// // draw max marks per column
//   Vector<Float> colsum(img.ncolumn());
//   for( uInt i=0; i<img.ncolumn(); i++ )
//     colsum(i) = sum( img.column(i) );
//   minMax(vmin,vmax,dum,dum,rowsum);
//   wh.resize();
//   wh = colsum > vmax-(vmax-vmin)*.5f;
//   for( uInt i=0; i<img.ncolumn(); i++ )
//     if( wh(i) )
//       pgp.arro((img.nrow()-.5)*1.02,i,img.ncolumn()-.5,i);
}

void RFFlagCube::plotAntAxis ( PGPlotterInterface &pgp,const Vector<uInt> &antnums,Bool yaxis )
{
  uInt nant = antnums.nelements();
  Vector<String> names(nant),antnames( chunk.antNames() );
  pgp.sch(1);
// find size of labels
  Float wmax=0,hmax=0;
  for( uInt i=0; i<nant; i++ )
  {
    names(i) = antnames( antnums(i) );
    Vector<Float> bbox( pgp.qtxt(0,0,0,0,names(i)) );
    Float w=bbox(3);
    hmax=bbox(7); 
    if( w>wmax )
      wmax=w;
  }
  if( yaxis ) // plot labels along Y axis
  {
    Vector<Float> win( pgp.qwin() );
    Float w0=(win(1)-win(0))/15,h0=.9;
    if( wmax>w0 || hmax>h0 )  // if bigger than available, then scale down to fit
      pgp.sch( max(w0/wmax,h0/hmax) );
    for( uInt i=0; i<nant; i++ )
      pgp.mtxt("LV",.5,(i+.5)/nant,1,names(i));
  }
  else // plot labels along X axis
  {
    Float w0=.9;
    if( wmax>w0 )
      pgp.sch(w0/wmax);
    for( uInt i=0; i<nant; i++ )
      pgp.mtxt("B",1.2,(i+.5)/nant,.5,names(i));
  }
  pgp.sch(1);
}

void RFFlagCube::plotIfrMap ( PGPlotterInterface &pgp,const Matrix<Float> &img,const LogicalVector &ifravail )
{
// plot antenna labels 
  Vector<uInt> antnums( num(ANT) );
  indgen(antnums);
  plotAntAxis(pgp,antnums,False);
  plotAntAxis(pgp,antnums,True);
// figure out size of characters to use
  pgp.sch(1);
  char s[16];
  sprintf(s,"%d",num(IFR)-1);
  Vector<Float> bbox( pgp.qtxt(0,0,0,0,s) );
  Float w=bbox(3),h=bbox(7); // normal w/h of labels
// available w/h is .9 by .9
  Float w0=.9,h0=.9;
  if( w>w0 || h>h0 )  // if bigger than available, then scale down to fit
    pgp.sch( max(w0/w,h0/h) );
// draw an ifr-number map
  Float halfheight = (pgp.qcs(4))(1)/2;
  Float imgmid=max(img)/2;
  for( uInt a1=0; a1<num(ANT); a1++ )
    for( uInt a2=0; a2<num(ANT); a2++ )
    {
      uInt ifr=chunk.antToIfr(a1,a2);
      if( ifravail(ifr) )
      {
        char s[16];
        sprintf(s,"%d",ifr);
        pgp.sci(img(a1,a2)<imgmid?1:0);
        pgp.ptxt(a1,a2-halfheight,0,.5,s);
      }
    }
  pgp.sch(1);
  pgp.sci(1);
}

Int RFFlagCube::numStatPlots (const RFChunkStats &chunk)
{
  Int count = 0;
  if( anyGT(flagrow,full_corrmask) ) // any row flags?
    count+=3; // 3 row flag plots
  if( anyGT(chunk.nfIfrTime(),0u) )   // any pixel flags?
  {
    count+=3; // 3 basic pixel flag plots
    if( chunk.num(CHAN)>1 )
      count+=2; // 3 freq-dependent pixel flag plots
  }
  return count;
}

void RFFlagCube::plotStats (PGPlotterInterface &pgp)
{
// plot only once on a given plotter
  if( report_plotted == &pgp )
    return;
  report_plotted = &pgp;
  String title( agent_names );
  
// get map of data availability by (ifr,time)
  LogicalMatrix rowmap( rowAvailabilityMap() );
  Matrix<uInt> rowcount( rowmap.shape() );
  convertArray(rowcount,rowmap);
// get counts of rows per ifr/per time
  Vector<uInt> row_per_ifr( rowcount.nrow() );
  for( uInt i=0; i<rowcount.nrow(); i++ )
    row_per_ifr(i) = sum( rowcount.row(i) );
  Vector<uInt> row_per_it( rowcount.ncolumn() );
  for( uInt i=0; i<rowcount.ncolumn(); i++ )
    row_per_it(i) = sum( rowcount.column(i) );
  Vector<uInt> row_per_ant( num(ANT),0u );
  Matrix<uInt> row_per_ant_time( num(ANT),num(TIME),0 );
  
// get start/end times and create label for time axis
  String timeaxis( "Time slot # (" +
        MVTime(chunk.startMJD()).string(MVTime::TIME|MVTime::CLEAN,6) + " through " +
        MVTime(chunk.endMJD()  ).string(MVTime::TIME|MVTime::CLEAN,6) +")" );
  
// SECTION 1: IFR (ANT-ANT) coverage maps  
// draw Antenna-Antenna row flag and pixel flag density image
  Vector<uInt> rowant(num(ANT),0u),pixant(num(ANT),0u);
  Matrix<Float> img1(num(ANT),num(ANT),0),img2(num(ANT),num(ANT),0);
  for( uInt ifr=0; ifr<num(IFR); ifr++ )
    if( row_per_ifr(ifr) )
    {
      uInt a1,a2;
      chunk.ifrToAnt(a1,a2,ifr);
      row_per_ant(a1) += row_per_ifr(ifr);
      if( a2!=a1 )
        row_per_ant(a2) += row_per_ifr(ifr);

      Double row_scale = 100./row_per_ifr(ifr),
            pix_scale = row_scale/num(CHAN);

      for( uInt it=0; it<num(TIME); it++ )
      {
        if( rowmap(ifr,it) )
        {
          row_per_ant_time(a1,it)++;
          if( a2!=a1 )
            row_per_ant_time(a2,it)++;
        }
        if( rowAgentFlagged(ifr,it) )
        {
          rowant(a1)++; 
          img1(a1,a2) += row_scale;
          if( a2!=a1 )
          {
            rowant(a2)++;
            img1(a2,a1) += row_scale;
          }
        }
        uInt n=chunk.nfIfrTime(ifr,it);
        if( n )
        {
          pixant(a1) += n;  
          img2(a1,a2) += n*pix_scale; 
          if( a2!=a1 )
          {
            pixant(a2) += n;
            img2(a2,a1) += n*pix_scale;
          }
        }
      }
    }
  if( sum(img1) )
  {
    plotImage(pgp,img1,"","",(title+": % rows flagged, by IFR").chars(),
	      True,1,1);
    plotIfrMap(pgp,img1,row_per_ifr!=0u);
  }
  if( sum(img2) )
  {
    plotImage(pgp,img2,"","",(title+": % pixels flagged, by IFR").chars(),
	      True,1,1);
    plotIfrMap(pgp,img2,row_per_ifr!=0u);
  }
  
// SECTION 2: Smth-IFR maps  
// setup antennas for which we have valid flag data 
  LogicalVector valid( rowant || pixant );
  uInt nval=0;
  for( uInt i=0; i<num(ANT); i++ )
    if( valid(i) )
      nval++;
  if( nval ) // do only if something is there
  {
    Vector<uInt> antnums(nval), // valid antenna numbers
       revant(num(ANT),9999u);   // reverse index 
    uInt ival=0;
    for( uInt i=0; i<num(ANT); i++ )
      if( valid(i) )
      {
        antnums(ival) = i;
        revant(i) = ival++;
      }
    if( nval<=30 ) // for relatively few antennas, use an antenna-based axis
    {
    // draw row flags image
      {
      Matrix<Float> img(num(TIME),nval*nval,0); 
      for( uInt ifr=0; ifr<num(IFR); ifr++ )
        if( row_per_ifr(ifr) )
        {
          uInt a1,a2; chunk.ifrToAnt(a1,a2,ifr);
          for( uInt it=0; it<num(TIME); it++ )
            if( rowAgentFlagged(ifr,it) )
            {
              uInt i1=revant(a1),i2=revant(a2);
              img(it,i1*nval+i2) = 1;
              img(it,i2*nval+i1) = 1;
            }
        }
      if( sum(img) )
      {
        plotImage(pgp,img,timeaxis.chars(),"",
		  (title+": Rows flagged, by time-IFR").chars(),False,0,nval);
        plotAntAxis(pgp,antnums,True);
      }
      }
    // draw IFR-Time image
      {
      Matrix<Float> img(num(TIME),nval*nval,0); 
      Double scale = 100./num(CHAN);
      for( uInt ifr=0; ifr<num(IFR); ifr++ )
        if( row_per_ifr(ifr) )
        {
          uInt a1,a2; chunk.ifrToAnt(a1,a2,ifr);
          for( uInt it=0; it<num(TIME); it++ )
          {
            uInt n = chunk.nfIfrTime(ifr,it);
            if( n )
            {
              uInt i1=revant(a1),i2=revant(a2);
              img(it,i1*nval+i2) += n*scale;
              if( a1!=a2 )
                img(it,i2*nval+i1) += n*scale;
            }
          }
        }
      if( sum(img) )
      {
        plotImage(pgp,img,timeaxis.chars(),"",
		  (title+": % pixels flagged, by time-IFR").chars(),
		  True,0,nval);
        plotAntAxis(pgp,antnums,True);
      }
      }
    // draw IFR-Channel image (skip if 1 channel only)
      if( num(CHAN)>1 )
      {
        Matrix<Float> img(num(CHAN),nval*nval,0); 
        for( uInt ifr=0; ifr<num(IFR); ifr++ )
          if( row_per_ifr(ifr) )
          {
            Double scale = 100./row_per_ifr(ifr);
            uInt a1,a2; chunk.ifrToAnt(a1,a2,ifr);
            for( uInt ich=0; ich<num(CHAN); ich++ )
            {
              uInt n = chunk.nfChanIfr(ich,ifr);
              if( n )
              {
                uInt i1=revant(a1),i2=revant(a2);
                img(ich,i1*nval+i2) += n*scale;
                if( a1!=a2 )
                  img(ich,i2*nval+i1) += n*scale;
              }
            }
          }
        plotImage(pgp,img,"Channel (frequency, MHz)","",
		  (title+": % pixels flagged, by channel-IFR").chars(),
		  True,0,nval,True);
        plotAntAxis(pgp,antnums,True);
      }
    }
    else // too many antennas - use a flat IFR axis
    {
    // draw row flags image
      {
      Matrix<Float> img(num(TIME),num(IFR),0); 
      for( uInt ifr=0; ifr<num(IFR); ifr++ )
        if( row_per_ifr(ifr) )
          for( uInt it=0; it<num(TIME); it++ )
            if( rowAgentFlagged(ifr,it) )
              img(it,ifr) = 1;
      if( sum(img) )
        plotImage(pgp,img,timeaxis.chars(),"IFR #",
		  (title+": Rows flagged, by time-IFR").chars(),False);
      }
    // draw IFR-Time image
      {
      Matrix<Float> img(num(TIME),num(IFR),0); 
      Double scale = 100./num(CHAN);
      for( uInt ifr=0; ifr<num(IFR); ifr++ )
        if( row_per_ifr(ifr) )
          for( uInt it=0; it<num(TIME); it++ )
            img(it,ifr) = chunk.nfIfrTime(ifr,it)*scale;
      if( sum(img) )
        plotImage(pgp,img,timeaxis.chars(),"IFR #",
		  (title+": % of pixels flagged, by time-IFR").chars());
      // draw IFR-Channel image
        if( num(CHAN)>1 )
        {
          Matrix<Float> img(num(CHAN),num(IFR),0); 
          for( uInt ifr=0; ifr<num(IFR); ifr++ )
            if( row_per_ifr(ifr) )
            {
              Double scale = 100./row_per_ifr(ifr);
              for( uInt ich=0; ich<num(CHAN); ich++ )
                img(ich,ifr) = chunk.nfChanIfr(ich,ifr)*scale;
            }
          plotImage(pgp,img,"Channel (frequency, MHz)","IFR #",
		    (title+": % of pixels flagged, by channel-IFR").chars(),
		    True,0,0,True);
        }
      }
    }
  
  // SECTION 3: Smth-Antenna maps  
  // draw Antenna-Time Slot row flag density image
    {
    Matrix<Float> img1(num(TIME),nval,0),img2(num(TIME),nval,0),img3(num(CHAN),nval,0);
    for( uInt ifr=0; ifr<num(IFR); ifr++ )
    {
      uInt a1,a2;
      chunk.ifrToAnt(a1,a2,ifr);
      
      for( uInt it=0; it<num(TIME); it++ )
        if( rowmap(ifr,it) )
        {
          Double scale1 = 100./row_per_ant_time(a1,it),
                 scale2 = 100./row_per_ant_time(a2,it);
          if( rowAgentFlagged(ifr,it) )
          {
            img1(it,revant(a1)) += scale1;
            if( a1!=a2 )
              img1(it,revant(a2)) += scale2;
          }
          uInt n=chunk.nfIfrTime(ifr,it);
          if( n )
          {
            img2(it,revant(a1)) += n*scale1/num(CHAN);
            if( a1!=a2 )
              img2(it,revant(a2)) += n*scale2/num(CHAN);
          }
        }
      if( num(CHAN)>1 )
        for( uInt ich=0; ich<num(CHAN); ich++ )
          if( row_per_ant(a1) )
          {
            uInt n=chunk.nfChanIfr(ich,ifr);
            if( n )
            {
              img3(ich,revant(a1)) += n*100./row_per_ant(a1);
              if( a1!=a2 )
                img3(ich,revant(a2)) += n*100./row_per_ant(a2);
            }
          }
    }
    if( sum(img1) )
    {
      plotImage(pgp,img1,timeaxis.chars(),"",
		(title+": % rows flagged, by time-antenna").chars(),
		True,0,1);
      plotAntAxis(pgp,antnums,True);
    }
    if( sum(img2) )
    {
      plotImage(pgp,img2,timeaxis.chars(),"",
		(title+": % pixels flagged, by time-antenna").chars(),
		True,0,1);
      plotAntAxis(pgp,antnums,True);
      if( num(CHAN)>1 )
      {
        plotImage(pgp,img3,"Channel (frequency, MHz)","",
		  (title+": % pixels flagged, by channel-antenna").chars(),
		  True,0,1,True);
        plotAntAxis(pgp,antnums,True);
      }
    }
    
    }
  } // endif( nval )
}

//template Array<RFlagWord> operator & ( const Array<RFlagWord> &arr,const RFlagWord &val);
//template LogicalArray maskBits  ( const Array<RFlagWord> &arr,const RFlagWord &val);



} //# NAMESPACE CASA - END


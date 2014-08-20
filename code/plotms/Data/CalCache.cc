//# CalCache.cc: Specialized PlotMSCache for filling CalTables
//# Copyright (C) 2009
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
//# $Id: $
#include <plotms/Data/CalCache.h>
#include <plotms/Data/PlotMSIndexer.h>

#include <casa/OS/Timer.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/Memory.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/Utilities/Sort.h>
#include <casa/Arrays/ArrayMath.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <synthesis/CalTables/CTColumns.h>
#include <synthesis/CalTables/CTMainColumns.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Threads/ThreadCommunication.h>
#include <tables/Tables/Table.h>

namespace casa {

CalCache::CalCache(PlotMSApp* parent):
  PlotMSCacheBase(parent),
  basis_("Circular")
{}

CalCache::~CalCache() {}


String CalCache::polname(Int ipol) {

  if (basis_=="Linear")
    return ( (ipol%2==0) ? String("X") : String("Y") );

  return ( (ipol%2==0) ? String("R") : String("L") );

  //  return String("?");
}




//*********************************
// protected method implementations

void CalCache::loadIt(vector<PMS::Axis>& loadAxes,
		      vector<PMS::DataColumn>& /*loadData*/,
		      ThreadCommunication* thread) {

  //  cout << "CalCache::loadIt" << endl;

  // Warn that averaging will be ignored
  if (averaging().anyAveraging())
    logWarn("CalCache::loadIt",
            "Ignoring averaging because this is a Cal Table");

  if (transformations().anyTransform())
    logWarn("CalCache::loadIt",
            "Ignoring transformations because this is a Cal Table");

  // Get various names, properties
  { 	 

    NewCalTable ct(NewCalTable::createCT(filename_,Table::Old,Table::Plain));

    parsAreComplex_ = ct.isComplex();
    basis_=ct.polBasis();

    ROCTColumns ctCol(ct);
    antnames_.resize();
    stanames_.resize();
    antstanames_.resize();
    fldnames_.resize();
    antnames_=ctCol.antenna().name().getColumn(); 	 
    stanames_=ctCol.antenna().station().getColumn(); 	 
    fldnames_=ctCol.field().name().getColumn(); 	 
    
    antstanames_=antnames_+String("@")+stanames_;

    nAnt_ = ctCol.antenna().nrow();
     
  } 	
 
  setUpCalIter(filename_,selection_,True,True,True);
    
  Vector<Int> nIterPerAve;

  // supports only channel averaging...    
  countChunks(*ci_p,thread);
  //    trapExcessVolume(pendingLoadAxes);
  loadCalChunks(*ci_p,loadAxes,thread);

  if (ci_p)
    delete ci_p;
  ci_p=NULL;
  
}

void CalCache::setUpCalIter(const String& ctname,
			    const PlotMSSelection& selection,
			    Bool readonly,
			    Bool /*chanselect*/,
			    Bool /*corrselect*/) {

  //cout << "CalCache::setUpCalIter" << endl;

  Int nsortcol(4);
  Block<String> columns(nsortcol);
  //  Double iterInterval(0.0);   TBD: is this ever needed?  (NCTI doesn't yet support it)
  columns[0]="SCAN_NUMBER";
  columns[1]="FIELD_ID";   
  columns[2]="SPECTRAL_WINDOW_ID";
  columns[3]="TIME";

   // Now open the MS, select on it, make the VisIter
  Table::TableOption tabopt(Table::Update);
  if (readonly) tabopt=Table::Old;

  // TBD: control locking here?
  NewCalTable ct(ctname,tabopt,Table::Plain), selct;

  // Apply selection
  Vector<Vector<Slice> > chansel;
  Vector<Vector<Slice> > corrsel;
  selection.apply(ct,selct,chansel,corrsel);
  
  // setup the volume meter
  //  vm_.reset();
  //  vm_= PMSCacheVolMeter(ms,averaging_,chansel,corrsel);

  if (readonly) {
    // Readonly version, for caching
    ci_p = new ROCTIter(selct,columns);
    wci_p =NULL;
  }
  else {
    // Writable, e.g. for flagging
    wci_p = new CTIter(selct,columns);
    ci_p = wci_p;  // const access
  }

  // Apply chan/corr selction
  //if (chanselect) rci_p->selectChannel(chansel);
  //if (corrselect) rci_p->selectCorrelation(corrsel);

}
      
void CalCache::countChunks(ROCTIter& ci,
			   ThreadCommunication* thread) {

  //  cout << "CalCache::countChunks" << endl;
  if (thread!=NULL) {
    thread->setStatus("Establishing cache size.  Please wait...");
    thread->setAllowedOperations(false,false,false);
  }


  ci.reset();

  // Count number of chunks.
  int chunk = 0;
  while (!ci.pastEnd()) {
    ++chunk;
    ci.next0();
  }

    /*
    if (thread!=NULL) {
      if (thread->wasCanceled()) {
	dataLoaded_=false;
	return;
      }
      else
	thread->setProgress(2);
    }
    */

  if(chunk != nChunk_) increaseChunks(chunk);
  
  //  cout << " Found " << nChunk_ << " chunks." << endl;

}


  /*
void MSCache::trapExcessVolume(map<PMS::Axis,Bool> pendingLoadAxes) {
  try {
    String s;
    s=vm_.evalVolume(pendingLoadAxes,netAxesMask(currentX_,currentY_));
    logLoad(s);
  } catch(AipsError& log) {
    // catch detected volume excess, clear the existing cache, and rethrow
    logLoad(log.getMesg());
    clear();
    stringstream ss;
    ss << "Please try selecting less data or averaging and/or" << endl
       << " 'force reload' (to clear unneeded cache items) and/or" << endl
       << " letting other memory-intensive processes finish.";
    throw(AipsError(ss.str()));
  }
}
  */ 


void CalCache::loadCalChunks(ROCTIter& ci,
			     const vector<PMS::Axis> loadAxes,
			     ThreadCommunication* thread) {

  //  cout << "CalCache::loadCalChunks" << endl;

  // permit cancel in progress meter:
  if(thread != NULL)
    thread->setAllowedOperations(false,false,true);
    
  logLoad("Loading chunks......");

  // Initialize the freq/vel calculator (in case we use it)
  //  vbu_=VisBufferUtil(vb);

  Int chunk = 0;
  chshapes_.resize(4,nChunk_);
  goodChunk_.resize(nChunk_);
  goodChunk_.set(False);
  double progress;

  // Reset iterator
  ci.reset();

  while (!ci.pastEnd()) {

      // If a thread is given, check if the user canceled.
      if(thread != NULL && thread->wasCanceled()) {
	dataLoaded_ = false;
	return;
      }
      
      // If a thread is given, update it.
      if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			    chunk % THREAD_SEGMENT == 0))
	thread->setStatus("Loading chunk " + String::toString(chunk) +
			  " / " + String::toString(nChunk_) + ".");
      
      // TBD: Support the following for CalTables? 
      // Adjust the visibility phase by phase-center shift
      //      vb.phaseCenterShift(transformations_.xpcOffset(),
      //			  transformations_.ypcOffset());


      // Current iteration as a NewCalTable object
      NewCalTable thisct(ci.table());

   /*
      cout << " #keywords = " << thisct.keywordSet().nfields() << endl;
      cout << " SPECTRAL_WINDOW? = " << boolalpha << thisct.keywordSet().isDefined("SPECTRAL_WINDOW") << endl;


      cout << " ch=" << chunk << " type=" << thisct.tableType() << flush;
   */
      // Discern npar/nchan shape
      ROCTMainColumns cmc(thisct);
      IPosition pshape(cmc.flag().shape(0));
      
      // cout << " pshape = " << pshape << flush;

      // Cache the data shapes
      chshapes_(0,chunk)=pshape[0];
      chshapes_(1,chunk)=pshape[1];
      chshapes_(2,chunk)=thisct.nrow();
      chshapes_(3,chunk)=nAnt_;
      goodChunk_(chunk)=True;

      for(unsigned int i = 0; i < loadAxes.size(); i++) {
	//	cout << PMS::axis(loadAxes[i]) << " ";
	loadCalAxis(ci, chunk, loadAxes[i]);
      }
      //cout << endl;
      chunk++;
      ci.next();
      
      // If a thread is given, update it.
      if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			    chunk % THREAD_SEGMENT == 0)) {
	progress = ((double)chunk+1) / nChunk_;
	thread->setProgress((unsigned int)((progress * 100) + 0.5));
      }
  }
}

  void CalCache::loadCalAxis(ROCTIter& cti, Int chunk, PMS::Axis axis) {
  
  switch(axis) {
    
  case PMS::SCAN: // assumes scan unique
    scan_(chunk) = cti.thisScan();
    break;
    
  case PMS::FIELD:
    field_(chunk) = cti.thisField();
    break;
    
  case PMS::TIME: { // assumes time unique 
    time_(chunk) = cti.thisTime();
    break;
  }
  /*        
    case PMS::TIME_INTERVAL: // assumes timeInterval unique in VB
        timeIntr_(chunk) = cti.interval()(0); 
	break;
  */
  case PMS::SPW:
    spw_(chunk) = cti.thisSpw(); 
    break;
  case PMS::CHANNEL: {
    cti.chan(*chan_[chunk]);
    break;
  }
    case PMS::FREQUENCY: {
      // TBD: Convert freq to desired frame
      cti.freq(*freq_[chunk]);
      (*freq_[chunk])/=1.0e9; // in GHz
      break;
    }

  /*
    case PMS::VELOCITY: {
      // Convert freq in the vb to velocity
      vbu_.toVelocity(*vel_[chunk],
		      vb,
		      transformations_.frame(),
		      MVFrequency(transformations_.restFreqHz()),
		      transformations_.veldef());
      (*vel_[chunk])/=1.0e3;  // in km/s
      break;
    }
  */

  case PMS::CORR:
    if (parsAreComplex()) 
      corr_[chunk]->resize(cti.flag().shape()(0));
    else
      corr_[chunk]->resize(cti.flag().shape()(0));
    indgen(*corr_[chunk]);
    break;
        
  case PMS::ANTENNA1:
    *antenna1_[chunk] = cti.antenna1(); 
    break;
  case PMS::ANTENNA2:
    *antenna2_[chunk] = cti.antenna2(); 
    break;
    
  case PMS::BASELINE: {
    Vector<Int> a1(cti.antenna1());
    Vector<Int> a2(cti.antenna2());
    baseline_[chunk]->resize(cti.nrow());
    Vector<Int> bl(*baseline_[chunk]);
    for (Int irow=0;irow<cti.nrow();++irow) {
      if (a1(irow)<0) a1(irow)=chshapes_(3,0);
      if (a2(irow)<0) a2(irow)=chshapes_(3,0);
      bl(irow)=(chshapes_(3,0)+1)*a1(irow) - (a1(irow) * (a1(irow) + 1)) / 2 + a2(irow);
    }
    break;
  }

  case PMS::GAMP:
  case PMS::AMP: {
    if (parsAreComplex()) 
      *amp_[chunk] = amplitude(cti.cparam());
    else
      *amp_[chunk] = cti.fparam();
    break;
  }
    
  case PMS::GPHASE:
  case PMS::PHASE: {
    if (parsAreComplex()) {
      *pha_[chunk] = phase(cti.cparam()); //*(180.0/C::pi);
      (*pha_[chunk])*=Float(180.0/C::pi);
    }
    else
      throw(AipsError("phase has no meaning for this table"));
    break;
  }

  case PMS::GREAL:
  case PMS::REAL: {
    if (parsAreComplex()) {
      *real_[chunk] = real(cti.cparam());
    }
    else
      throw(AipsError("real has no meaning for this table"));
    break;
  }
  case PMS::GIMAG:
  case PMS::IMAG: {
    if (parsAreComplex()) {
      *imag_[chunk] = imag(cti.cparam());
    }
    else
      throw(AipsError("imag has no meaning for this table"));
    break;
  }

  case PMS::DELAY:{
	  if ( !parsAreComplex()){
		  *par_[chunk] = cti.fparam();
	  }
	  else {
		  throw(AipsError( "delay has no meaning for this table"));
	  }
	  break;
  }

  case PMS::OPAC: {

	if ( !parsAreComplex()){
		*par_[chunk] = cti.fparam();
	}
	else {
		throw(AipsError( "opacity has no meaning for this table"));
	}
    break;
  }
  case PMS::SWP: {
    *par_[chunk] = cti.fparam()(Slice(0,2,2),Slice(),Slice());
    break;
  }
  case PMS::TSYS: {
    throw(AipsError("Tsys NYI"));
    break;
  }
                
  case PMS::FLAG:
    *flag_[chunk] = cti.flag();
    break;
 /*
    case PMS::WT: {
      *wt_[chunk] = cti.weightMat();
      break;
    }
    case PMS::AZ0:
    case PMS::EL0: {
      Vector<Double> azel;
      cti.azel0Vec(cti.time()(0),azel);
      az0_(chunk) = azel(0);
      el0_(chunk) = azel(1);
      break;
    }
    case PMS::HA0: 
      ha0_(chunk) = cti.hourang(cti.time()(0))*12/C::pi;  // in hours
      break;
    case PMS::PA0: {
      pa0_(chunk) = cti.parang0(cti.time()(0))*180.0/C::pi; // in degrees
      if (pa0_(chunk)<0.0) pa0_(chunk)+=360.0;
      break;
    }
 */
  case PMS::ANTENNA: {
    antenna_[chunk]->resize(nAnt_);
    indgen(*antenna_[chunk]);
    break;
  }

 /*
    case PMS::AZIMUTH:
    case PMS::ELEVATION: {
      Matrix<Double> azel;
      cti.azelMat(cti.time()(0),azel);
      *az_[chunk] = azel.row(0);
      *el_[chunk] = azel.row(1);
      break;
    }
    case PMS::PARANG:
      *parang_[chunk] = cti.feed_pa(cti.time()(0))*(180.0/C::pi);  // in degrees
      break;
    case PMS::ROW: {
      *row_[chunk] = cti.rowIds();
      break;
    }
 */
  default:
    throw(AipsError("Axis choice not supported for Cal Tables"));
    break;
  }
}




void CalCache::flagToDisk(const PlotMSFlagging& flagging,
			 Vector<Int>& flchunks, Vector<Int>& flrelids, 
			 Bool flag, PlotMSIndexer* indexer, int dataIndex ) {
  
  // Sort the flags by chunk:
  Sort sorter;
  sorter.sortKey(flchunks.data(),TpInt);
  sorter.sortKey(flrelids.data(),TpInt);
  Vector<uInt> order;
  uInt nflag;
  nflag = sorter.sort(order,flchunks.nelements());

  stringstream ss;

  // Make the VisIterator writable, with selection revised as appropriate
  Bool selectchan(netAxesMask_[dataIndex](1) && !flagging.channel());
  Bool selectcorr(netAxesMask_[dataIndex](0) && !flagging.corrAll());

  // Set up a write-able CTIter (ci_p also points to it)
  setUpCalIter(filename_,selection_,False,selectchan,selectcorr);

  ci_p->reset();

  Int iflag(0);
  for (Int ichk=0;ichk<nChunk_;++ichk) {
    
    if (ichk!=flchunks(order[iflag]) &&
	!ci_p->pastEnd())
      // nothing to flag this chunk, just advance
      ci_p->next();


    else {
      
      // This chunk requires flag-setting
      
      Int ifl(iflag);
      
      // Get bits we need from the table
      Cube<Bool> ctflag;
      Vector<Int> channel,a1,a2;
      ci_p->flag(ctflag);
      ci_p->chan(channel);
      ci_p->antenna1(a1);
      ci_p->antenna2(a2);

      Int npar=ctflag.shape()(0);
      Int nchan=channel.nelements();
      Int nrow=ci_p->nrow();

      if (True) {
	Int currChunk=flchunks(order[iflag]);
	Double time=getTime(currChunk,0);
	Double cttime=ci_p->time()(0);
	Int spw=Int(getSpw(currChunk,0));
	Int ctspw=ci_p->thisSpw();
	Int field=Int(getField(currChunk,0));
	Int ctfld=ci_p->thisField();
	ss << "Time diff:  " << time-cttime << " " << time  << " " << cttime << "\n";
	ss << "Spw diff:   " << spw-ctspw   << " " << spw   << " " << ctspw  << "\n"; 
	ss << "Field diff: " << field-ctfld << " " << field << " " << ctfld  << "\n";
      }

      // Apply all flags in this chunk to this VB
      ifl=iflag;
      while (ifl<Int(nflag) && flchunks(order[ifl])==ichk) {

	Int currChunk=flchunks(order[ifl]);
	Int irel=flrelids(order[ifl]);

	Slice par,chan,bsln;
	
	// Set flag range on par axis:
	if (netAxesMask_[dataIndex](0) && !flagging.corrAll()) {
	  // A specific single par
	  Int ipar=indexer->getIndex1000(currChunk,irel);
	  par=Slice(ipar,1,1);
	}
	else
	  // all on par axis
	  par=Slice(0,npar,1);

	// Set Flag range on channel axis:
	if (netAxesMask_[dataIndex](1) && !flagging.channel()) {
	  // A single specific channel 
	  Int ichan=indexer->getIndex0100(currChunk,irel);
	  chan=Slice(ichan,1,1);
	}
	else 
	  // Extend to all channels
	  chan=Slice(0,nchan,1);
	  
	// Set Flags on the baseline axis:
	Int thisA1=Int(getAnt1(currChunk,indexer->getIndex0010(currChunk,irel)));
	Int thisA2=Int(getAnt2(currChunk,indexer->getIndex0010(currChunk,irel)));
	if (netAxesMask_[dataIndex](2) &&
	    !flagging.antennaBaselinesBased() &&
	    thisA1>-1 ) {
	  // i.e., if baseline is an explicit data axis, 
	  //       full baseline extension is OFF
	  //       and the first antenna in the selected point is > -1
	  
	  // Do some variety of detailed per-baseline flagging
	  for (Int irow=0;irow<nrow;++irow) {
	      
	    if (thisA2>-1) {
	      // match a baseline exactly
	      if (a1(irow)==thisA1 &&
		  a2(irow)==thisA2) {
		ctflag(par,chan,Slice(irow,1,1))=flag;
		break;  // found the one baseline, escape from for loop
	      }
	    }
	    else {
	      // either antenna matches the one specified antenna
	      //  (don't break because there will be more than one)
	      if (a1(irow)==thisA1 ||
		  a2(irow)==thisA1) {
		ctflag(par,chan,Slice(irow,1,1))=flag;
	      }
	    }
	  }
	}
	else {
	  // Set flags for all baselines, because the plot
	  //  is ordinarily implicit in baseline, we've turned on baseline
	  //  extension, or we've avaraged over all baselines
	  bsln=Slice(0,nrow,1);
	  ctflag(par,chan,bsln)=flag;
	} 
	
	++ifl;
      }
      
      // Put the flags back into the MS
      wci_p->setflag(ctflag);
      
      // Advance to the next vb
      if (!ci_p->pastEnd())
	ci_p->next();
      else
	// we are done, so escape chunk loop
	break;

      // step over the flags we've just done
      iflag=ifl;
      
      // Escape if we are already finished
      if (uInt(iflag)>=nflag) break;
      
    } // flaggable chunk
    
  } // ichk

  // Delete the VisIter so lock is released
  if (wci_p)
    delete wci_p;
  wci_p=NULL;
  ci_p=NULL;

  logFlag(ss.str());

}

}

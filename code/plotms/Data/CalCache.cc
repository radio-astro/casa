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
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Threads/ThreadCommunication.h>

#include <casa/OS/Timer.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/Memory.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/Utilities/Sort.h>
#include <casa/Arrays/ArrayMath.h>
#include <tables/Tables/Table.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <synthesis/CalTables/CTColumns.h>
#include <synthesis/CalTables/CTMainColumns.h>
#include <synthesis/MeasurementComponents/VisCalGlobals.h>

using namespace casacore;

namespace casa {

CalCache::CalCache(PlotMSApp* parent):
  PlotMSCacheBase(parent),
  basis_("unknown"),
  divZero_(False)
{
}

CalCache::~CalCache() {}


String CalCache::polname(Int ipol) {
  if (polnRatio_) return "/";
  if (basis_=="Linear")
    return ( (ipol%2==0) ? String("X") : String("Y") );
  else if (basis_=="Circular")
    return ( (ipol%2==0) ? String("R") : String("L") );
  else // "unknown"
    return ( (ipol%2==0) ? String("0") : String("1") );
}


//*********************************
// protected method implementations

void CalCache::loadIt(vector<PMS::Axis>& loadAxes,
		      vector<PMS::DataColumn>& /*loadData*/,
		      ThreadCommunication* thread) {

  // update cal type
  setFilename(filename_);

  if (calType_=="BPOLY" || calType_=="GSPLINE" || 
      calType_[0]=='A' || calType_[0]=='M' || calType_[0]=='X')
    throw AipsError("Cal table type " + calType_ + " is unsupported in plotms.  Please continue to use plotcal.");
  if (calType_=="KAntPos Jones")
    throw AipsError("Cannot plot " + calType_ + " tables.");

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
    try {
        parsAreComplex_ = ct.isComplex();
    } catch (AipsError & err) {
        throw AipsError("Plotms does not support calibration tables in the old (pre-CASA 3.4) format.  Please continue to use plotcal.");
    }

    basis_=ct.polBasis();

    ROCTColumns ctCol(ct);
    antnames_.resize();
    stanames_.resize();
    antstanames_.resize();
    fldnames_.resize();
    positions_.resize();

    antnames_=ctCol.antenna().name().getColumn(); 	 
    stanames_=ctCol.antenna().station().getColumn(); 	 
    antstanames_=antnames_+String("@")+stanames_;
    fldnames_=ctCol.field().name().getColumn();
    positions_ = ctCol.antenna().position().getColumn();    

    nAnt_ = ctCol.antenna().nrow();
     
  } 	

  setUpCalIter(filename_,selection_, True,True,True);
    
  // supports only channel averaging...    
  vector<PMS::DataColumn> loadData(loadAxes.size());
  for (uInt i=0; i<loadData.size(); ++i) 
    loadData[i] = PMS::DEFAULT_DATACOLUMN;
  countChunks(*ci_p, loadAxes, loadData, thread);
  //    trapExcessVolume(pendingLoadAxes);
  loadCalChunks(*ci_p,loadAxes,thread);

  if (ci_p)
    delete ci_p;
  ci_p=NULL;
}

void CalCache::setUpCalIter(const String& ctname,
			    PlotMSSelection& selection,
			    Bool readonly,
			    Bool /*chanselect*/,
			    Bool /*corrselect*/) {

  // check for invalid caltypes for ratio plots
  if (selection.corr() == "/") {
    if (calType_ == "BPOLY" || calType_[0] == 'T' || calType_[0] == 'F')
      throw(AipsError("Polarization ratio plots not supported for " + calType_ + " tables."));
    polnRatio_ = true;
  } else {
    polnRatio_ = false;
  }

  Int nsortcol(4);
  Block<String> columns(nsortcol);
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

  // Apply chan/corr selection
  //if (chanselect) rci_p->selectChannel(chansel);
  //if (corrselect) rci_p->selectCorrelation(corrsel);

}
      
void CalCache::countChunks(ROCTIter& ci,
               vector<PMS::Axis>& loadAxes,
		       vector<PMS::DataColumn>& loadData,
			   ThreadCommunication* thread) {

  if (thread!=NULL) {
    thread->setStatus("Establishing cache size.  Please wait...");
    thread->setAllowedOperations(false,false,false);
  }

  // Count number of chunks.
  int chunk = 0;
  ci.reset();
  while (!ci.pastEnd()) {
    ++chunk;
    ci.next0();
  }

  setCache(chunk, loadAxes, loadData);
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

      // Discern npar/nchan shape
      IPosition pshape(ci.flag().shape());
      size_t nPol;
      String pol = selection_.corr();
      if (pol=="" || pol=="RL" || pol=="XY") { // no selection
        nPol = pshape[0];
        // half the data for EVLASWP table is swp, half is tsys
        if (calType_.contains("EVLASWP")) nPol = pshape[0]/2;
        pol = "";
      } else { // poln selection using calParSlice
        String paramAxis = toVisCalAxis(PMS::AMP);
        if (polnRatio_)  // pick one!
            nPol = getParSlice(paramAxis, "R").length();
        else 
            nPol = getParSlice(paramAxis, pol).length();
      }

      // Cache the data shapes
      chshapes_(0,chunk)=nPol;
      chshapes_(1,chunk)=pshape[1];
      chshapes_(2,chunk)=ci.nrow();
      chshapes_(3,chunk)=nAnt_;
      goodChunk_(chunk)=True;

      for(unsigned int i = 0; i < loadAxes.size(); i++) {
        loadCalAxis(ci, chunk, loadAxes[i], pol);
      }
        chunk++;
        ci.next();
      
        // If a thread is given, update it.
        if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
            chunk % THREAD_SEGMENT == 0)) {
            progress = ((double)chunk+1) / nChunk_;
            thread->setProgress((unsigned int)((progress * 100) + 0.5));
        }
  }
  if (divZero_)
    logWarn("CalCache::loadIt", "Caught divide-by-zero exception in ratio plots; result(s) set to 1.0 and flagged");
}

  void CalCache::loadCalAxis(ROCTIter& cti, Int chunk, PMS::Axis axis, String pol) {
    Slice parSlice1 = Slice();
    Slice parSlice2 = Slice();
    if (PMS::axisNeedsCalSlice(axis)) {
        String calAxis = toVisCalAxis(axis);
        if (polnRatio_) {
            parSlice1 = getParSlice(calAxis, "R");
            parSlice2 = getParSlice(calAxis, "L");
        } else {
            parSlice1 = getParSlice(calAxis, pol);
        }
    }

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

  case PMS::CORR: {
    corr_[chunk]->resize(chshapes_(0,chunk));
    if (pol=="" || pol=="RL" || pol=="XY") {
        indgen(*corr_[chunk]);
    } else if (pol== "R" || pol=="X") { 
      corr_[chunk]->resize(1);
      corr_[chunk]->set(0);
    } else if (pol== "L" || pol=="Y") { 
      corr_[chunk]->resize(1);
      corr_[chunk]->set(1);
    } else if (pol=="/") {
      corr_[chunk]->resize(1);
      corr_[chunk]->set(-1); // ???
    }
    break;
  }
        
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
    if (parsAreComplex()) {
        Cube<Complex> cArray = cti.cparam();
        if (polnRatio_) {
            Array<Float> ampRatio = amplitude(cArray(parSlice1, Slice(), Slice()) / cArray(parSlice2, Slice(), Slice()));
            checkRatioArray(ampRatio, chunk);
            *amp_[chunk] = ampRatio;
        } else {
            *amp_[chunk] = amplitude(cArray(parSlice1, Slice(), Slice()));
        }
    } else {
        Cube<Float> fArray = cti.fparam();
        if (polnRatio_) {
            Array<Float> ampRatio = fArray(parSlice1, Slice(), Slice()) / fArray(parSlice2, Slice(), Slice());
            checkRatioArray(ampRatio, chunk);
            *amp_[chunk] = ampRatio;
        } else {        
            *amp_[chunk] = fArray(parSlice1, Slice(), Slice());
        }
        if (calType_[0] == 'F') // TEC table
            (*amp_[chunk]) /= Float(1e+16);
    }
    break;
  }
    
  case PMS::GPHASE:
  case PMS::PHASE: {
    if (parsAreComplex()) {
        Cube<Complex> cArray = cti.cparam();
        if (polnRatio_) {
            Array<Float> phaseRatio = phase(cArray(parSlice1, Slice(), Slice()) / cArray(parSlice2, Slice(), Slice()));
            checkRatioArray(phaseRatio, chunk);
            *pha_[chunk] = phaseRatio;
        } else {
            *pha_[chunk] = phase(cArray(parSlice1, Slice(), Slice()));
        }
        (*pha_[chunk]) *= Float(180.0/C::pi);
    } else {
        throw(AipsError("phase has no meaning for this table"));
    }
    break;
  }

  case PMS::GREAL:
  case PMS::REAL: {
    if (parsAreComplex()) {
        Cube<Complex> cArray = cti.cparam();
        if (polnRatio_) {
            Array<Float> realRatio = real(cArray(parSlice1, Slice(), Slice()) / cArray(parSlice2, Slice(), Slice()));
            checkRatioArray(realRatio, chunk);
            *real_[chunk] = realRatio;
        } else {        
            *real_[chunk] = real(cArray(parSlice1, Slice(), Slice()));
        }
    } else {  // allow float for single dish cal tables
        Cube<Float> fArray = cti.fparam();
        if (polnRatio_) {
            Array<Float> ampRatio = fArray(parSlice1, Slice(), Slice()) / fArray(parSlice2, Slice(), Slice());
            checkRatioArray(ampRatio, chunk);
            *real_[chunk] = ampRatio;
        } else {        
            *real_[chunk] = fArray(parSlice1, Slice(), Slice());
        }
    }
    break;
  }

  case PMS::GIMAG:
  case PMS::IMAG: {
    if (parsAreComplex()) {
        Cube<Complex> cArray = cti.cparam();
        if (polnRatio_) {
            Array<Float> imagRatio = imag(cArray(parSlice1, Slice(), Slice()) / cArray(parSlice2, Slice(), Slice()));
            checkRatioArray(imagRatio, chunk);
            *imag_[chunk] = imagRatio;
        } else {        
            *imag_[chunk] = imag(cArray(parSlice1, Slice(), Slice()));
        }
    } else
        throw(AipsError("imag has no meaning for this table"));
    break;
  }

  case PMS::DELAY:{
    if (!parsAreComplex()) {
        Cube<Float> fArray = cti.fparam();
        if (polnRatio_) {
            Array<Float> delayRatio = fArray(parSlice1, Slice(), Slice()) - fArray(parSlice2, Slice(), Slice());
            checkRatioArray(delayRatio, chunk);
            *par_[chunk] = delayRatio;
        } else {
           *par_[chunk] = fArray(parSlice1, Slice(), Slice());
        }
    } else
        throw(AipsError( "delay has no meaning for this table"));
    break;
  }

  case PMS::OPAC: {
    if (!parsAreComplex() && calType_.contains("Opac")) {
        Cube<Float> fArray = cti.fparam();
        *par_[chunk] = fArray(parSlice1, Slice(), Slice());
    } else
		throw(AipsError( "opacity has no meaning for this table"));
    break;
  }

  case PMS::SWP: {   // "SPGAIN" in plotcal
    if ( !parsAreComplex() && calType_.contains("EVLASWPOW")) {
        Cube<Float> fArray = cti.fparam();
        if (polnRatio_) {
            Array<Float> swpRatio = fArray(parSlice1, Slice(), Slice()) / fArray(parSlice2, Slice(), Slice());
            checkRatioArray(swpRatio, chunk);
            *par_[chunk] = swpRatio;
        } else {
           *par_[chunk] = fArray(parSlice1, Slice(), Slice());
        }
    } else
        throw( AipsError( "SwPower has no meaning for this table"));
    break;
  }

  case PMS::TSYS: {
	if ( !parsAreComplex() && (calType_.contains("EVLASWPOW") || calType_.contains("TSYS"))) {
        Cube<Float> fArray = cti.fparam();
        if (polnRatio_) {
            Array<Float> tsysRatio = fArray(parSlice1, Slice(), Slice()) / fArray(parSlice2, Slice(), Slice());
            checkRatioArray(tsysRatio, chunk);
            *par_[chunk] = tsysRatio;
        } else {
           *par_[chunk] = fArray(parSlice1, Slice(), Slice());
        }
    } else
        throw(AipsError( "Tsys has no meaning for this table"));
    break;
  }

  case PMS::SNR: {
    if (polnRatio_) {
        Array<Float> snrRatio = cti.snr()(parSlice1, Slice(), Slice()) / cti.snr()(parSlice2, Slice(), Slice());
        checkRatioArray(snrRatio, chunk);
        *snr_[chunk] = snrRatio;
    } else {
        *snr_[chunk] = cti.snr()(parSlice1, Slice(), Slice());
    }
    break;
  }

   case PMS::TEC: {
	if ( !parsAreComplex() && calType_[0]=='F') {
        Cube<Float> fArray = cti.fparam();
        *par_[chunk] = (fArray(parSlice1, Slice(), Slice()))/1e+16;
    } else
        throw(AipsError( "TEC has no meaning for this table"));
    break;
  }

                
  case PMS::FLAG:
    if (polnRatio_)
        *flag_[chunk] = cti.flag()(parSlice1, Slice(), Slice()) | cti.flag()(parSlice2, Slice(), Slice());
    else
        *flag_[chunk] = cti.flag()(parSlice1, Slice(), Slice());
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
  case PMS::OBSERVATION: {
    (*obsid_[chunk]).resize(1);
    *obsid_[chunk] = cti.thisObs();
    break;
  }
  case PMS::INTENT: {
    // metadata axis that always gets loaded so don't want to throw exception
    break;
  }
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

      // Apply poln selection
      Int npar;
      String pol = selection_.corr();
      if (pol=="" || pol=="RL" || pol=="XY" || pol=="/") { // both axes
        npar = ctflag.shape()(0);
      } else { // poln selection using calParSlice
        String paramAxis = toVisCalAxis(PMS::FLAG);
        npar = getParSlice(paramAxis, pol).length();
      }    
      Int nchan = channel.nelements();
      Int nrow = ci_p->nrow();

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

	Slice par1,chan,bsln;
	Slice par2 = Slice();

	// Set flag range on par axis:
	if (netAxesMask_[dataIndex](0) && !flagging.corrAll()) {
	  // A specific single par
      if (pol=="" || pol=="RL" || pol=="XY") {  // flag both axes
	    Int ipar=indexer->getIndex1000(currChunk,irel);
	    par1 = Slice(ipar,1,1);
      } else if (polnRatio_) {
        par1 = getParSlice(toVisCalAxis(PMS::AMP), "R");
        par2 = getParSlice(toVisCalAxis(PMS::AMP), "L");
      } else {
        par1 = getParSlice(toVisCalAxis(PMS::AMP), pol);
      }
	}
	else
	  // all on par axis
	  par1 = Slice(0,npar,1);

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
		ctflag(par1,chan,Slice(irow,1,1)) = flag;
        if (par2.length() > 0)
		    ctflag(par2,chan,Slice(irow,1,1)) = flag;
		break;  // found the one baseline, escape from for loop
	      }
	    }
	    else {
	      // either antenna matches the one specified antenna
	      //  (don't break because there will be more than one)
	      if (a1(irow)==thisA1 ||
		  a2(irow)==thisA1) {
		ctflag(par1,chan,Slice(irow,1,1)) = flag;
        if (par2.length() > 0)
		    ctflag(par2,chan,Slice(irow,1,1)) = flag;
	      }
	    }
	  }
	}
	else {
	  // Set flags for all baselines, because the plot
	  //  is ordinarily implicit in baseline, we've turned on baseline
	  //  extension, or we've avaraged over all baselines
	  bsln=Slice(0,nrow,1);
	  ctflag(par1,chan,bsln) = flag;
      if (par2.length() > 0)
	    ctflag(par2,chan,bsln) = flag;
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

String CalCache::toVisCalAxis(PMS::Axis axis) {
    switch (axis) {
        // FLAG and SNR have same shape as AMP 
        // and should be sliced the same way
        case PMS::AMP:
        case PMS::GAMP:
        case PMS::FLAG:
        case PMS::SNR:
            if (calType_.contains("EVLASWP")) return "GAINAMP";
            if (calType_.contains("TSYS")) return "TSYS";
            if (calType_[0] == 'K') return "DELAY";
            if (calType_[0] == 'F') return "TEC";
            if (calType_ == "TOpac") return "OPAC";
            return "AMP";
            break;
        case PMS::PHASE:
        case PMS::GPHASE:
            return "PHASE";
            break;
        case PMS::REAL:
        case PMS::GREAL:
            return "REAL";
            break;
        case PMS::IMAG:
        case PMS::GIMAG:
            return "IMAG";
            break;
        default:
            return PMS::axis(axis);
            break;
    }
}

Slice CalCache::getParSlice(String axis, String polnSel) {
    Slice parSlice = Slice();
    try {
        parSlice = viscal::calParSlice(filename_, axis, polnSel);
    } catch(AipsError& err) {
        if (err.getMesg().contains("Unsupported value type")) {
            // Message a bit vague at top level, add some explanation
            String errMsg = err.getMesg() + ". Invalid axis or polarization selection for cal table type.";
            throw(AipsError(errMsg));
        } else { // unsupported cal type
            throw(AipsError(err));
        }
    }
    return parSlice;
}

void CalCache::setFilename(String filename) { 
    filename_ = filename;
    NewCalTable ct(NewCalTable::createCT(filename_,Table::Old,Table::Plain));
    calType_= ct.tableInfo().subType();
}

void CalCache::checkRatioArray(Array<Float>& array, Int chunk) {
    Cube<Float> ratioCube;
    ratioCube.reference(array);
    Cube<Bool> flags;
    flags.reference(*flag_[chunk]);

    IPosition cubeShape = ratioCube.shape();
    for (uInt i=0; i<cubeShape[0]; ++i) {
        for (uInt j=0; j<cubeShape[1]; ++j) {
            for (uInt k=0; k<cubeShape[2]; ++k) {
                if (isInf(ratioCube(i,j,k))) {
                    ratioCube(i,j,k) = 1.0;
                    flags(i,j,k) = True;
                    divZero_ = True;
                }
            }
        }
    }
}

}

//# PlotCalCache.cc: Cal cache for plotms
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
#include <plotms/Data/PlotCalCache.h>

#include <casa/OS/Timer.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Sort.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ExprNode.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

namespace casa {

PlotCalCache::PlotCalCache(PlotMS* parent) :
  PlotMSCache(parent),
  tab_p(NULL),
  seltab_p(NULL),
  iter_p(NULL)
{ }

PlotCalCache::~PlotCalCache() {
  deleteFreqInfo();
  closeCal();  // Just to be sure
}

void PlotCalCache::load(const vector<PMS::Axis>& axes,
			const vector<PMS::DataColumn>& data,
			const String& filename,
			const PlotMSSelection& selection,
			const PlotMSAveraging& averaging,
			const PlotMSTransformations& transformations,
			PlotMSCacheThread* thread) {

  // Maintain access to this msname, selection, & averager, because we'll
  // use it if/when we flag, etc.
  calname_ = filename;

  // We (will) still support selection (soon)
  selection_ = selection;

  // No averaging or transformations supported (yet?)
  if (averaging.anyAveraging()) {
    stringstream noavewarn;
    noavewarn << "AVERAGING IS NOT SUPPORTED FOR CALTABLES AT THIS TIME; IGNORING.\n";
    logWarn("PlotCalCache::load",noavewarn.str());
  }
  averaging_ = PlotMSAveraging();

  if (transformations.anyTransform()) {
    stringstream notranswarn;
    notranswarn << "TRANSFORMATIONS ARE NOT SUPPORTED FOR CALTABLES AT THIS TIME; IGNORING.\n";
    logWarn("PlotCalCache::load", notranswarn.str());
  }
  transformations_ = PlotMSTransformations();


  // Catch unsupported axes here:
  for(unsigned int i = 0; i < axes.size(); i++) {
    switch (axes[i]) {
    case PMS::TIME_INTERVAL:
    case PMS::SCAN:
    case PMS::VELOCITY:
    case PMS::ANTENNA2:
    case PMS::BASELINE:
    case PMS::CORR:
    case PMS::FLAG_ROW:
    case PMS::UVDIST:
    case PMS::UVDIST_L:
    case PMS::U:
    case PMS::V:
    case PMS::W:
    case PMS::AZ0:
    case PMS::EL0:
    case PMS::HA0:
    case PMS::PA0:
    case PMS::ANTENNA:
    case PMS::AZIMUTH:
    case PMS::ELEVATION:
    case PMS::PARANG:
    case PMS::ROW: {
      stringstream noaxiserr;
      noaxiserr << "Axis: '" << PMS::axis(axes[i]) << "' is not supported for caltables at this time.\n";
      throw(AipsError(noaxiserr.str()));
      break;
    }
    default:
      break;
      // OK to proceed
    }
  }

  stringstream ss;
  ss << "Caching cal table for the new plot: " 
       << PMS::axis(axes[1]) << "("<< axes[1] << ") vs. " 
       << PMS::axis(axes[0]) << "(" << axes[0] << ")...\n";

  logLoad(ss.str());


  // Calculate which axes need to be loaded; those that have already been
  // loaded do NOT need to be reloaded (assuming that the rest of PlotMS has
  // done its job and cleared the cache if the underlying MS/selection has
  // changed).
  vector<PMS::Axis> loadAxes; vector<PMS::DataColumn> loadData;
  
  // Check meta-data.
  for(unsigned int i = 0; i < N_METADATA; i++) {
    if(!loadedAxes_[METADATA[i]]) {
      loadAxes.push_back(METADATA[i]);
      loadData.push_back(PMS::DEFAULT_DATACOLUMN);
    }
  }
        
  // Check given axes.  Should only be added to load list if: 1) not
  // already in load list, 2) not loaded, or 3) loaded but with different
  // data column (if applicable).


  bool found; PMS::Axis axis; PMS::DataColumn dc; 
  for(unsigned int i = 0; i < axes.size(); i++) {
   found = false;
    axis = axes[i];
    

    // if data vector is not the same length as axes vector, assume
    // default data column
    dc = PMS::DEFAULT_DATACOLUMN;

    // 1)
    for(unsigned int j = 0; !found && j < loadAxes.size(); j++)
      if(loadAxes[j] == axis) found = true;
    if(found) continue;
    
    // 2)
    if(!loadedAxes_[axis]) {
      loadAxes.push_back(axis);
      loadData.push_back(dc);
    }
    
    // 3)
    else if(PMS::axisIsData(axis) && dc != loadedAxesData_[axis]) {
      loadAxes.push_back(axis);
      loadData.push_back(dc);
    }
  }

  if(loadAxes.size() == 0) return; // nothing to be loaded
  
  // Now Load data.
    
  // Setup the selected Cal table 
  openCal(calname_,selection,True);  // True==readonly

  // Allocate and load
  countCalChunks();
  loadCalChunks(loadAxes,loadData,thread);
    
  // Remember # of VBs per Average
  // TBD: rename this variable?
  nVBPerAve_.resize(nChunk_);
  nVBPerAve_.set(1);
  
  // Update loaded axes.
  for(unsigned int i = 0; i < loadAxes.size(); i++) {
    axis = loadAxes[i];
    loadedAxes_[axis] = true;
    if(PMS::axisIsData(axis)) loadedAxesData_[axis] = loadData[i];
  }
  
  dataLoaded_ = true;


  // Close the cal table (and assoc MS!)
  closeCal();

  logLoad("Finished loading.");
}

void PlotCalCache::flagToDisk(const PlotMSFlagging& flagging,
			      Vector<Int>& flchunks, Vector<Int>& flrelids, 
			      Bool flag) {

  // Sort the flags by chunk:
  Sort sorter;
  sorter.sortKey(flchunks.data(),TpInt);
  sorter.sortKey(flrelids.data(),TpInt);
  Vector<uInt> order;
  uInt nflag;
  nflag = sorter.sort(order,flchunks.nelements());

  stringstream ss;

  // Handle chan/corr flagging scope
  //  Bool selectchan(netAxesMask_(1) && !flagging.channel());
  //  Bool selectcorr(netAxesMask_(0) && !flagging.corrAll());

  // Establish a scope in which the CalTable object is created and destroyed
  {

  // Open the current table with current selection
  openCal(calname_,selection_,False);  // True==readonly

  
  Table tabchunk;
  Int ichk=0;
  uInt iflag=0;
  while (!iter_p->pastEnd()) {

    tabchunk=iter_p->table();
    ArrayColumn<Bool> flagcol(tabchunk,"FLAG");
    Cube<Bool> oflag=flagcol.getColumn();
    
    IPosition flsh=oflag.shape();
    Int ncorr=flsh(0);
    Int nchan=flsh(1);
    Int nrow=flsh(2);

    // If the current chunk has flags, apply them
    while (flchunks(order[iflag])==ichk) {

      currChunk_=flchunks(order[iflag]);
      irel_=flrelids(order[iflag]);
      
      Slice corr,chan,bsln;
      
      // Set flag range on correlation axis:
      if (netAxesMask_(0) && !flagging.corrAll()) {
	// A specific single correlation
	Int icorr=(irel_%icorrmax_(currChunk_));  // the index
	corr=Slice(icorr,1,1);
      }
      else
	corr=Slice(0,ncorr,1);
      
      // Set Flag range on channel axis:
      if (netAxesMask_(1) && !flagging.channel()) {
	// A single specific channel 
	Int ichan=(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_);
	chan=Slice(ichan,1,1);
      }
      else 
	// Extend to all channels
	chan=Slice(0,nchan,1);
      
      // Set Flags on the antenna axis:
      // TBD: handle baseline-based?  (see PlotMSCache)
      if (netAxesMask_(2) && !flagging.antennaBaselinesBased()) {
	// A single specific row
	Int irow=(irel_/nperbsln_(currChunk_))%ichanbslnmax_(currChunk_);
	bsln=Slice(irow,1,1);
      }
      else
	bsln=Slice(0,nrow,1);

      // Set the flag
      oflag(corr,chan,bsln)=flag;
	
      // advance to the next flag
      ++iflag;

    }

    // Done setting this chunk; put them back
    flagcol.putColumn(oflag);

    // Force to disk
    tabchunk.flush();
    
    // Advance iterator while there are flags to do
    if (iflag<nflag) {
      iter_p->next();
      ++ichk;
    }
    else
      // Finished flagging, so quit iterating
      break;

  }

  // Close the cal table
  closeCal();

  } // caltale dtor scope

  logFlag(ss.str());

}


void PlotCalCache::openCal(const String& calname,
			   const PlotMSSelection& selection,
			   Bool readonly) {

  // Make a table object
  if(!Table::isReadable(calname)) 
    throw(AipsError("Table "+calname+" does not exist."));

  // Interpret the cal type
  //  String upType;
  // upType=calTableType(calname);


  if (readonly)
    tab_p=new Table(calname);  // read-only
  else
    tab_p=new Table(calname,Table::Update);  // read/write
    

  if(!tab_p->tableInfo().type().contains("Calibration"))
    throw(AipsError("Table "+calname+" is not a calibration table."));

  // Extract some useful info from the CAL_DESC_ID subtable, etc.
  {
    Table caldesctab(calname_+"/CAL_DESC");

    nCDI_=caldesctab.nrow();

    // The associated MS name (needed for meta info, selection, etc.)
    ROScalarColumn<String> msNameCol(caldesctab,"MS_NAME");
    msname_= msNameCol(0);

    // Spws mapped from CAL_DESC_IDs
    ROArrayColumn<Int> spwCol(caldesctab,"SPECTRAL_WINDOW_ID");
    Array<Int> spws=spwCol.getColumn();
    spwFromCDI_ = spws.nonDegenerate();

    // The channel coverage
    ROArrayColumn<Int> numchanCol(caldesctab,"NUM_CHAN");
    nchanFromCDI_=numchanCol.getColumn().nonDegenerate();

    ROArrayColumn<Int> chanrangeCol(caldesctab,"CHAN_RANGE");
    Array<Int> chanrange=chanrangeCol.getColumn();
    IPosition blc(4,0,0,0,0), trc(4,0,0,0,nCDI_-1);
    startFromCDI_=chanrange(blc,trc).nonDegenerate();

    // The reference time
    ROScalarColumn<Double> timeCol(*tab_p,"TIME");
    refTime_p=min(timeCol.getColumn());
    refTime_p=86400.0*floor(refTime_p/86400.0);

    // The channel frequencies from the MS:
    getFrequencyInfo();

  }

  // Interpret and apply selection
  selectCal(selection);

  // Generate the iterator
  Block<String> itercol(2);
  itercol[0] = "CAL_DESC_ID";
  itercol[1] = "TIME";
  iter_p = new TableIterator(*seltab_p,itercol);

}

void PlotCalCache::closeCal() {

  // Delete the transient table objects:
  if (iter_p) delete iter_p;
  iter_p=NULL;
  if (seltab_p && seltab_p!=tab_p) delete seltab_p;
  seltab_p=NULL;
  if (tab_p) delete tab_p;
  tab_p=NULL;

}

void PlotCalCache::selectCal(const PlotMSSelection& selection) {


  String antenna=selection.antenna();
  String field=selection.field();
  String spw=selection.spw();
  String time=selection.timerange();
  String poln=selection.corr();
  
  Vector<Int> antId; 
  Vector<Int> fldId;
  Vector<Int> spwId;
  
  Bool antSel=False;
  Bool fldSel=False;
  Bool spwSel=False;
  Bool timeSel=False;
  
  Bool noMS_p(False);

  if (msname_=="" || !Table::isReadable(msname_)) {
    cout << "Note: Either your CalTable pre-dates name-based selection, or" << endl;
    cout << "      does not (yet) support selection, or the MS associated" << endl;
    cout << "      with this cal table does not exist.  All antennas," << endl;
    cout << "      fields, spws are being selected for plotting." << endl;

    noMS_p=True;
  }
  else {

    noMS_p=False;
    
    // Use MSSelection-assistance
    antId=getAntIdx(antenna);
    if (antenna.length()>0 && antId.nelements()==0)
      throw(AipsError("Specified antenna(s) select no calibration solutions."));
    if(antId.nelements()>0 && antId(0)!=-1)
      antSel=True;
    
    fldId=getFieldIdx(field);
    if (field.length()>0 && fldId.nelements()==0)
      throw(AipsError("Specified field(s) select no calibration solutions."));
    if(fldId.nelements()>0 && fldId(0)!=-1)
      fldSel=True;
    
    spwId=getSpwIdx(spw);
    if (spw.length()>0 && spwId.nelements()==0)
      throw(AipsError("Specified spw(s) select no calibration solutions."));
    if(spwId.nelements()>0 && spwId(0)!=-1) 
      spwSel=True;
    
    if(time.length()>0)
      timeSel=True;

    // Polarization selection (includes '/')
    //    polType_p=upcase(poln);

  }
  
  if(antSel || fldSel || spwSel || timeSel){
    
    TableExprNode condition;
    
    if(antSel){
      String col="ANTENNA1";
      condition=tab_p->col(col).in(antId);
    }
    if(fldSel){
      String col="FIELD_ID";
      if(condition.isNull())
        condition=tab_p->col(col).in(fldId);
      else
        condition=condition && tab_p->col(col).in(fldId);
    }
    if(spwSel){

      String col="CAL_DESC_ID";
      Vector<Int> caldescids=getCalDescIds(spwId);

      if (caldescids.nelements() > 0) {
        if(condition.isNull())
          condition=tab_p->col(col).in(caldescids);
        else
          condition=condition && tab_p->col(col).in(caldescids);
      }
      else {
        throw(AipsError("Selected spws not found in cal table."));
      }
    }
    
    if (timeSel) {
      
      Matrix<Double> timeRng=getTimeRange(time);
      
      if (timeRng.nrow()>0) {
        
        TableExprNode timeTEN;
        
        for (uInt itm=0;itm<timeRng.ncolumn();++itm) {
          
          // Form single time selection
          TableExprNode thistimeTEN;
          Vector<Double> thistime(timeRng.column(itm));
          if (thistime(0)!=thistime(1)) {
            thistimeTEN=(tab_p->col("TIME") >= min(thistime));
            thistimeTEN=thistimeTEN && (tab_p->col("TIME") <= max(thistime));
          }
          else {
            // Handle single time specification (within +/- 30s
            thistimeTEN=(tab_p->col("TIME") >= thistime(0)-30.0);
            thistimeTEN=thistimeTEN && (tab_p->col("TIME") <=thistime(0)+30);
          }
          
          // Combine with global time selection
          if (!thistimeTEN.isNull()) {
            if (timeTEN.isNull())
              timeTEN=thistimeTEN;
            else
              timeTEN=timeTEN || thistimeTEN;
          }
          
        }
        
        // Combine with global selection
        if (!timeTEN.isNull()) {
          if (condition.isNull())
            condition = timeTEN;
          else
            condition = condition && timeTEN;
        }
      }
    }
      // Only if 
    if (!condition.isNull()) {

      seltab_p=new Table();
      *seltab_p=(*tab_p)(condition);
      seltab_p->rename(tab_p->tableName()+".plotCal", Table::New);
      seltab_p->flush();
      seltab_p->markForDelete();

      if(seltab_p->nrow()==0) {
        seltab_p=NULL;
        throw(AipsError("Combined selection selects nothing."));
        return;
      }
    }
  }
  else 
    // Select whole cal table for plotting
    seltab_p = tab_p;
  
  return;
}


      
void PlotCalCache::countCalChunks() {

  // Sum slots for each spw

  Int nChunk=0;
  while (!iter_p->pastEnd()) {
    ++nChunk;
    iter_p->next();
  }
  
  // Inflate the cache to the correct number of chunks
  if (nChunk != nChunk_)
    increaseChunks(nChunk);

}
 
void PlotCalCache::loadCalChunks(const vector<PMS::Axis> loadAxes,
				 const vector<PMS::DataColumn> loadData,
				 PlotMSCacheThread* thread) {
    
  logLoad("Loading calibration chunks......");

  // For each spw and slot, load required info
  chshapes_.resize(4,nChunk_);
  cdi_.resize(nChunk_,0);

  double progress;

  iter_p->reset();
  Int chunk = 0;
  Table tabchunk;
  while (!iter_p->pastEnd()) {

    if(thread != NULL && thread->wasCanceled()) {
      dataLoaded_ = false;
      return;
    }
    
    // If a thread is given, update it.
    if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			  chunk % THREAD_SEGMENT == 0))
      thread->setStatus("Loading chunk " + String::toString(chunk) +
			" / " + String::toString(nChunk_) + ".");
    
    // The current iteration's Table
    tabchunk=iter_p->table();

    ROScalarColumn<Int> col(tabchunk,"CAL_DESC_ID");
    cdi_(chunk)=col(0);

    // Cache the data shapes
    ROArrayColumn<Bool> fl(tabchunk,"FLAG");
    IPosition cellshape=fl.shape(0);

    chshapes_(0,chunk)=cellshape(0);
    chshapes_(1,chunk)=cellshape(1);  // should use CDI info here!
    chshapes_(2,chunk)=tabchunk.nrow();
    chshapes_(3,chunk)=tabchunk.nrow();

    // Loop over required axes
    for(unsigned int i = 0; i < loadAxes.size(); i++) {
      loadCalAxis(tabchunk, chunk, loadAxes[i]);
    }
      
    // Advance iter
    iter_p->next();
    chunk++;

    // If a thread is given, update it.
    if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			  chunk % THREAD_SEGMENT == 0)) {
      progress = ((double)chunk+1) / nChunk_;
      thread->setProgress((unsigned int)((progress * 100) + 0.5));
    }
  }
}






void PlotCalCache::reportMeta(Double x, Double y,stringstream& ss) {

  // Use MS versio for now (OK, I think)
  PlotMSCache::reportMeta(x,y,ss);


/* cal-specific version NYI

  ss << "Scan=" << getScan() << " ";
  ss << "Field=" << getField() << " ";
  ss << "Time=" << MVTime(getTime()/C::day).string(MVTime::YMD,7) << " ";
  ss << "BL=";

  Int ant1=Int(getAnt1());
  if (!netAxesMask_(2) || ant1<0)
    ss << "*-";
  else
    ss << ant1 << "-";

  Int ant2=Int(getAnt2());
  if (!netAxesMask_(2) || ant2<0)
    ss << "* ";
  else
    ss << ant2 << " ";

  Int spw=Int(getSpw());
  ss << "Spw=";
  if (spw<0)
    ss << "* ";
  else
    ss << spw << " ";

  ss << "Chan=";
  if (netAxesMask_(1)) {
    if (averaging_.channel() && averaging_.channelValue()>1) {
      Int ichan=(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_);
      Int& lochan=chanAveBounds_p(spw)(ichan,0);
      Int& hichan=chanAveBounds_p(spw)(ichan,1);
      ss << " new:<" << lochan << "~" << hichan  << "> ";

    }
    else 
      ss << getChan() << " ";
  }
  else
    ss << "*  ";

  ss << "Freq=";
  if (netAxesMask_(1))
    ss << getFreq() << " ";
  else
    ss << "*        ";

  ss << "Corr=";
  if (netAxesMask_(0))
    ss << getCorr() << " ";
  else
    ss << "* ";
  ss << "X=" << x << " ";
  ss << "Y="  << y << " ";
  ss << "(" << (currChunk_ > 0 ? (nPoints_(currChunk_-1)+irel_) : irel_) << "/";
  ss << currChunk_ << "/" << irel_ << ")";
*/
}

void PlotCalCache::loadCalAxis(Table& tab, Int ichunk, PMS::Axis axis) {

    switch(axis) {

    case PMS::SCAN: {
      scan_(ichunk) = -1;
      break;
    }
    case PMS::FIELD: {
      ROScalarColumn<Int> col(tab,"FIELD_ID");
      field_(ichunk) = col(0);
      break;
    }
    case PMS::TIME: {
      ROScalarColumn<Double> col(tab,"TIME");
      time_(ichunk) = col(0); 
      break;
    }
    case PMS::SPW: {
      spw_(ichunk) = spwFromCDI_(cdi_(ichunk));
      break;
    }
    case PMS::TIME_INTERVAL: {
      timeIntr_(ichunk) = -1.0;
      break;
    }
    case PMS::CHANNEL: {
      chan_[ichunk]->resize(chshapes_(1,ichunk));  
      indgen(*chan_[ichunk]);  // TBD: offset by startchannel
      break;
    }
    case PMS::FREQUENCY: {
      // Convert freq to desired frame
      *freq_[ichunk] = *freqFromCDI_[cdi_(ichunk)];
      break;
    }
    case PMS::CORR: {
      corr_[ichunk]->resize(chshapes_(0,ichunk));
      indgen(*corr_[ichunk]);
      break;
    }
    case PMS::ANTENNA1: {
      ROScalarColumn<Int> col(tab,"ANTENNA1");
      Vector<Int> ant1=col.getColumn();
      *antenna1_[ichunk] = ant1;
      break;
    }
    case PMS::ANTENNA2: {
      //      ROScalarColumn<Int> col(tab,"REF_ANT");
      //      Vector<Int> ant2=col.getColumn();
      antenna2_[ichunk]->resize(chshapes_(2,ichunk));
      antenna2_[ichunk]->set(-1);
      break;
    }
    case PMS::BASELINE: {
      baseline_[ichunk]->resize(chshapes_(2,ichunk));
      indgen(*baseline_[ichunk]);
      break;
    }
    case PMS::AMP: {
      ROArrayColumn<Complex> col(tab,"GAIN");
      *amp_[ichunk] = amplitude(col.getColumn());
      break;
    }
    case PMS::PHASE: {
      ROArrayColumn<Complex> col(tab,"GAIN");
      *pha_[ichunk] = phase(col.getColumn())*180.0/C::pi;
      break;
    }
    case PMS::REAL: {
      ROArrayColumn<Complex> col(tab,"GAIN");
      *real_[ichunk] = real(col.getColumn());
      break;
    }
    case PMS::IMAG: {
      ROArrayColumn<Complex> col(tab,"GAIN");
      *imag_[ichunk] = imag(col.getColumn());
      break;
    }
    case PMS::FLAG: {
      ROArrayColumn<Bool> col(tab,"FLAG");
      *flag_[ichunk] = Cube<Bool>(col.getColumn());
      break;
    }
    case PMS::FLAG_ROW: {
      // Trivial (useless)
      *flagrow_[ichunk] = Vector<Bool>(chshapes_(2,ichunk),False);
      break;
    }
    case PMS::ROW: {
      *row_[ichunk] = tab.rowNumbers();
      break;
    }
    // Unsupported axes (so far)
    case PMS::VELOCITY:
    case PMS::UVDIST:
    case PMS::U:
    case PMS::V:
    case PMS::W:
    case PMS::UVDIST_L:
    case PMS::AZ0:
    case PMS::EL0:
    case PMS::HA0: 
    case PMS::PA0:
    case PMS::ANTENNA:
    case PMS::AZIMUTH:
    case PMS::ELEVATION:
    case PMS::PARANG:
      //    case PMS::WEIGHT:
    default:
      throw(AipsError("Axis choice NYI"));
      break;
    }
}



Vector<Int> PlotCalCache::getAntIdx(const String& antenna) {

  // TBD: handle baseline-based terms
    
  if (msname_!="" && Table::isReadable(msname_)) {
    MeasurementSet ms(msname_);
    MSSelection mssel;

    // Antenna-based case is easy
    mssel.setAntennaExpr(antenna);
    return mssel.getAntenna1List(&ms);
  }
  else {
    return Vector<Int>();
  }
  
}

// Interpret field indices (MSSelection)
Vector<Int> PlotCalCache::getFieldIdx(const String& field) {
    
  if (msname_!="" && Table::isReadable(msname_)) {
    MeasurementSet ms(msname_);
    MSSelection mssel;
    mssel.setFieldExpr(field);
    return mssel.getFieldList(&ms);
  }
  else {
    return Vector<Int>();
  }
  
}

// Get cal_desc indices (via MSSelection on spws)
Vector<Int> PlotCalCache::getSpwIdx(const String& spw) {

  if (msname_!="" && Table::isReadable(msname_)) {
    MeasurementSet ms(msname_);
    MSSelection mssel;
    mssel.setSpwExpr(spw);
    return mssel.getSpwList(&ms);
  }
  else {
    return Vector<Int>();
  }
  
}

Vector<Int> PlotCalCache::getCalDescIds(const Vector<Int> selspws) {
    
  Int ncdi=spwFromCDI_.nelements();

  Vector<Int> allcds(ncdi);
  Vector<Bool> cdmask(ncdi,False);
  indgen(allcds);

  for (Int icd=0;icd<ncdi;++icd) {
    cdmask(icd)=False;
    for (Int ispw=0;ispw<Int(selspws.nelements());++ispw)
      cdmask(icd)|=(spwFromCDI_(icd)==selspws(ispw));
  }
  
  return allcds(cdmask).getCompressedArray();
  
}

// Interpret field indices (MSSelection)
Matrix<Double> PlotCalCache::getTimeRange(const String& time) {
  
  if (msname_!="" && Table::isReadable(msname_)) {
    MeasurementSet ms(msname_);
    MSSelection mssel;
    mssel.setTimeExpr(time);
    return mssel.getTimeList(&ms);
  }
  else {
    return Matrix<Double>();
  }
  
}

void PlotCalCache::getFrequencyInfo() {

  // Delete prior freq info
  deleteFreqInfo();

  MeasurementSet ms(msname_);
  ROArrayColumn<Double> 
    chanfreqcol(ROMSSpWindowColumns(ms.spectralWindow()).chanFreq());

  freqFromCDI_.resize(nCDI_);
  for (Int icd=0;icd<nCDI_;++icd) {
    freqFromCDI_[icd] = new Vector<Double>();
    Slice sl(startFromCDI_(icd),nchanFromCDI_(icd),1);
    *freqFromCDI_[icd] = chanfreqcol(spwFromCDI_(icd))(sl);
    *freqFromCDI_[icd] /=1.0e9;  // in GHz
  }


}

void PlotCalCache::deleteFreqInfo() {

  Int n=freqFromCDI_.nelements();
  for (Int i=0;i<n;++i)
    if (freqFromCDI_[i]) delete freqFromCDI_[i];

  freqFromCDI_.resize(0,True);

}

}  // namespace casa

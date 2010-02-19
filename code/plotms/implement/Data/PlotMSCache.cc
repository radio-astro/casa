//# PlotMSCache.cc: Data cache for plotms.
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
#include <plotms/Data/PlotMSCache.h>

#include <casa/OS/Timer.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Sort.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <scimath/Mathematics/FFTServer.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <plotms/Data/PlotMSVBAverager.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>

namespace casa {

const PMS::Axis PlotMSCache::METADATA[] =
    { PMS::TIME, PMS::TIME_INTERVAL, PMS::FIELD, PMS::SPW, PMS::SCAN,
      PMS::ANTENNA1, PMS::ANTENNA2, PMS::BASELINE, 
      PMS::CHANNEL, PMS::CORR, PMS::FREQUENCY,
      PMS::FLAG, PMS::FLAG_ROW };
const unsigned int PlotMSCache::N_METADATA = 13;

bool PlotMSCache::axisIsMetaData(PMS::Axis axis) {
    for(unsigned int i = 0; i < N_METADATA; i++)
        if(METADATA[i] == axis) return true;
    return false;
}

const unsigned int PlotMSCache::THREAD_SEGMENT = 10;

                                           
PlotMSCache::PlotMSCache(PlotMS* parent):
  plotms_(parent),
  nAnt_(0),
  nChunk_(0),
  nPoints_(),
  nTotalPoints_(0),
  nUnFlagPoints_(0),
  nFlagPoints_(0),
  refTime_p(0.0),
  minX_(0),
  maxX_(0),
  minY_(0),
  maxY_(0),
  time_(),
  timeIntr_(),
  field_(),
  spw_(),
  scan_(),
  dataLoaded_(false),
  axesSet_(false)
{

  if (False) {

    Vector<Complex> x(16,Complex(1.0));
    cout << "x = " << x << endl;
    ArrayLattice<Complex> xal(x);
    LatticeFFT::cfft(xal);
    cout << "x = " << x << endl;

  }
      



    // Set up loaded axes to be initially empty, and set up data columns for
    // data-based axes.
    const vector<PMS::Axis>& axes = PMS::axes();
    for(unsigned int i = 0; i < axes.size(); i++) {
        loadedAxes_[axes[i]] = false;
        if(PMS::axisIsData(axes[i]))
            loadedAxesData_[axes[i]]= PMS::DEFAULT_DATACOLUMN;
    }    
}

PlotMSCache::~PlotMSCache() {
  // Deflate everything
  deleteCache();
}

// increase the number of chunks we can store
void PlotMSCache::increaseChunks(Int nc) {

  Int oldnChunk=nChunk_;

  if (nc==0) {   // no guidance
    if (nChunk_<1) // currently empty
      nChunk_=32;
    else
      // Double it
      nChunk_*=2;
  }
  else
    // Add requested number
    nChunk_+=nc;

  // Resize, copying existing contents
  scan_.resize(nChunk_,True);
  time_.resize(nChunk_,True);
  timeIntr_.resize(nChunk_,True);
  field_.resize(nChunk_,True);
  spw_.resize(nChunk_,True);
  chan_.resize(nChunk_,False,True);
  freq_.resize(nChunk_,False,True);
  vel_.resize(nChunk_,False,True);
  corr_.resize(nChunk_,False,True);

  row_.resize(nChunk_,False,True);
  antenna1_.resize(nChunk_,False,True);
  antenna2_.resize(nChunk_,False,True);
  baseline_.resize(nChunk_,False,True);
  uvdist_.resize(nChunk_,False,True);
  uvdistL_.resize(nChunk_,False,True);
  u_.resize(nChunk_,False,True);
  v_.resize(nChunk_,False,True);
  w_.resize(nChunk_,False,True);

  amp_.resize(nChunk_,False,True);
  pha_.resize(nChunk_,False,True);
  real_.resize(nChunk_,False,True);
  imag_.resize(nChunk_,False,True);
  flag_.resize(nChunk_,False,True);
  flagrow_.resize(nChunk_,False,True);

  az0_.resize(nChunk_,True);
  el0_.resize(nChunk_,True);
  ha0_.resize(nChunk_,True);
  pa0_.resize(nChunk_,True);
  
  antenna_.resize(nChunk_,False,True);
  az_.resize(nChunk_,False,True);
  el_.resize(nChunk_,False,True);
  parang_.resize(nChunk_,False,True);


  plmask_.resize(nChunk_,False,True);
  
  // Construct (empty) pointed-to Vectors/Arrays
  for (Int ic=oldnChunk;ic<nChunk_;++ic) {
    row_[ic] = new Vector<uInt>();
    antenna1_[ic] = new Vector<Int>();
    antenna2_[ic] = new Vector<Int>();
    baseline_[ic] = new Vector<Int>();
    uvdist_[ic] = new Vector<Double>();
    uvdistL_[ic] = new Matrix<Double>();
    u_[ic] = new Vector<Double>();
    v_[ic] = new Vector<Double>();
    w_[ic] = new Vector<Double>();
    freq_[ic] = new Vector<Double>();
    vel_[ic] = new Vector<Double>();
    chan_[ic] = new Vector<Int>();
    corr_[ic] = new Vector<Int>();
    amp_[ic] = new Array<Float>();
    pha_[ic] = new Array<Float>();
    real_[ic] = new Array<Float>();
    imag_[ic] = new Array<Float>();
    flag_[ic] = new Array<Bool>();
    flagrow_[ic] = new Vector<Bool>();
    antenna_[ic] = new Vector<Int>();
    az_[ic] = new Vector<Double>();
    el_[ic] = new Vector<Double>();
    parang_[ic] = new Vector<Float>();

    plmask_[ic] = new Array<Bool>();
  }

  // Zero the cumulative count
  nPoints_.resize(nChunk_);
  nPoints_=0;


}

void PlotMSCache::clear() {
    deleteCache();
    refTime_p=0.0;
}

void PlotMSCache::load(const vector<PMS::Axis>& axes,
		       const vector<PMS::DataColumn>& data,
		       const String& msname,
		       const PlotMSSelection& selection,
		       const PlotMSAveraging& averaging,
		       const PlotMSTransformations& transformations,
		       PlotMSCacheThread* thread) {
    // TBD: 
    // o Should we have ONE PtrBlock to a list of Records, each of which
    //    we fill with desired data/info, rather than private data for
    //    every possible option?  (Keys become indices, etc., Eventual 
    //    disk paging?)
    // o Partial appends? E.g., if we have to go back and get something 
    //    that we didn't get the first time around.  Use of Records as above
    //    may make this easier, too.

      // need a way to keep track of whether:
      // 1) we already have the metadata loaded
      // 2) the underlying MS has changed, requiring a reloading of metadata

  // Maintain access to this msname, selection, & averager, because we'll
  // use it if/when we flag, etc.
  msname_ = msname;
  selection_ = selection;
  averaging_ = averaging;
  transformations_ = transformations;

  logLoad(transformations_.summary());
  logLoad(averaging_.summary());


  // Check if scr cols present
  Bool scrcolOk(False);

  {
    const ColumnDescSet cds=Table(msname).tableDesc().columnDescSet();
    scrcolOk=cds.isDefined("CORRECTED_DATA");

  }

  stringstream ss;
  ss << "Caching for the new plot: " 
       << PMS::axis(axes[1]) << "("<< axes[1] << ") vs. " 
       << PMS::axis(axes[0]) << "(" << axes[0] << ")...\n";

  logLoad(ss.str());

  if (!scrcolOk) 
    logLoad("NB: Scratch columns not present; will use DATA exclusively.");



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
    if(i < data.size() && scrcolOk) dc = data[i];

    // 1)
    for(unsigned int j = 0; !found && j < loadAxes.size(); j++)
      if(loadAxes[i] == axis) found = true;
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
    
  // Setup the selected Visiter (getting sort right to support averaging)
  setUpVisIter(msname,selection,True,True,True);
  ROVisIterator& viter(*rvi_p);

  if (averaging_.channel()) 
    viter.setChanAveBounds(averaging_.channelValue(),chanAveBounds_p);
  else 
    viter.setChanAveBounds(-1.0,chanAveBounds_p);
    
  // Remember how many antennas there are
  //   (should remove this)
  nAnt_ = viter.numberAnt();

  // TBD: Consolidate count/loadChunks methods?
  
  Vector<Int> nIterPerAve;
  if ( (averaging.time() && averaging.timeValue()>0.0) ||
       averaging.baseline() ||
       averaging.antenna() ||
       averaging.spw() ) {
    
    countChunks(viter,nIterPerAve,averaging);
    loadChunks(viter,averaging,nIterPerAve,
	       loadAxes,loadData,thread);
    
  }
  else {

    // supports only channel averaging...    
    countChunks(viter);
    loadChunks(viter,loadAxes,loadData,averaging,thread);
    
  }
  
  // Remember # of VBs per Average
  nVBPerAve_.resize();
  if (nIterPerAve.nelements()>0)
    nVBPerAve_ = nIterPerAve;
  else {
    nVBPerAve_.resize(nChunk_);
    nVBPerAve_.set(1);
  }
  
  // Update loaded axes.
  for(unsigned int i = 0; i < loadAxes.size(); i++) {
    axis = loadAxes[i];
    loadedAxes_[axis] = true;
    if(PMS::axisIsData(axis)) loadedAxesData_[axis] = loadData[i];
  }
  
  dataLoaded_ = true;

  if (rvi_p)
    delete rvi_p;
  wvi_p=NULL;
  rvi_p=NULL;

  logLoad("Finished loading.");
}

 
void PlotMSCache::loadChunks(ROVisibilityIterator& vi,
			     const vector<PMS::Axis> loadAxes,
			     const vector<PMS::DataColumn> loadData,
			     const PlotMSAveraging& averaging,
			     PlotMSCacheThread* thread) {
    
  logLoad("Loading chunks......");
  VisBuffer vb(vi);

  // Initialize the freq/vel calculator (in case we use it)
  vbu_=VisBufferUtil(vb);

  Int chunk = 0;
  chshapes_.resize(4,nChunk_);
  double progress;
  for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
    for(vi.origin(); vi.more(); vi++) {
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
      

      // Force data I/O so that shifting averaging will work, if nec.
      forceVBread(vb,loadAxes,loadData);

      // Adjust the visibility phase by phase-center shift
      vb.phaseCenterShift(transformations_.xpcOffset(),
			  transformations_.ypcOffset());

      
      // Do channel averaging, if required
      if (averaging.channel() && averaging.channelValue()>0.0) {
	  // Delegate actual averaging to the VisBuffer:
	  vb.channelAve(chanAveBounds_p(vb.spectralWindow()));
      }
      
      // Cache the data shapes
      chshapes_(0,chunk)=vb.nCorr();
      chshapes_(1,chunk)=vb.nChannel();
      chshapes_(2,chunk)=vb.nRow();
      chshapes_(3,chunk)=vi.numberAnt();
      
      for(unsigned int i = 0; i < loadAxes.size(); i++) {
	//	cout << PMS::axis(loadAxes[i]) << " ";
	loadAxis(vb, chunk, loadAxes[i], loadData[i]);
      }
      //cout << endl;
      chunk++;
      
      // If a thread is given, update it.
      if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			    chunk % THREAD_SEGMENT == 0)) {
	progress = ((double)chunk+1) / nChunk_;
	thread->setProgress((unsigned int)((progress * 100) + 0.5));
      }
    }
  }
  
}

void PlotMSCache::loadChunks(ROVisibilityIterator& vi,
			     const PlotMSAveraging& averaging,
			     const Vector<Int>& nIterPerAve,
			     const vector<PMS::Axis> loadAxes,
			     const vector<PMS::DataColumn> loadData,
			     PlotMSCacheThread* thread) {
  
  logLoad("Loading chunks with averaging.....");

  Bool verby(False);

  VisBuffer vb(vi);

  // Initialize the freq/vel calculator (in case we use it)
  vbu_=VisBufferUtil(vb);

  chshapes_.resize(4,nChunk_);
  double progress;
  vi.originChunks();
  vi.origin();


  Double time0=86400.0*floor(vb.time()(0)/86400.0);
  for (Int chunk=0;chunk<nChunk_;++chunk) {

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
      
    // Arrange to accumulate many VBs into one
    PlotMSVBAverager pmsvba(vi.numberAnt(),vi.existsWeightSpectrum());

    // Tell averager if we are averaging baselines together
    pmsvba.setBlnAveraging(averaging.baseline());
    pmsvba.setAntAveraging(averaging.antenna());

    // Turn on scalar averaging, if set (else Vector Ave will be done)
    pmsvba.setScalarAve(averaging.scalarAve());

    // Sort out which data to read
    discernData(loadAxes,loadData,pmsvba);

    stringstream ss;
    if (verby) ss << chunk << "----------------------------------\n";

    for (Int iter=0;iter<nIterPerAve(chunk);++iter) {

      // Force read on required stuff
      forceVBread(vb,loadAxes,loadData);
      
      if (verby) {
	ss << "ck=" << chunk << " vb=" << iter << " (" << nIterPerAve(chunk) << ");  " 
	     << "sc=" << vb.scan()(0) << " "
	     << "time=" << vb.time()(0)-time0 << " "
	     << "fl=" << vb.fieldId() << " "
	     << "sp=" << vb.spectralWindow() << " ";
      }

      // Adjust the visibility phase by phase-center shift
      vb.phaseCenterShift(transformations_.xpcOffset(),
			  transformations_.ypcOffset());

      // Do channel averaging, if required
      if (averaging.channel() && averaging.channelValue()>0.0) {
	// Delegate actual averaging to the VisBuffer:
	vb.channelAve(chanAveBounds_p(vb.spectralWindow()));
      }
      
      // Accumulate into the averager
      pmsvba.accumulate(vb);
      
      // Advance to next VB
      vi++;

      if (verby) ss << " next VB ";
      
      
      if (!vi.more() && vi.moreChunks()) {
	// go to first vb in next chunk
	if (verby) ss << "  stepping VI";
	vi.nextChunk();
	vi.origin();
      }
      if (verby) ss << "\n";
    }
    
    if (verby) logLoad(ss.str());


    // Finalize the averaging
    pmsvba.finalizeAverage();

    // The averaged VisBuffer
    VisBuffer& avb(pmsvba.aveVisBuff());

    // Cache the data shapes
    chshapes_(0,chunk)=avb.nCorr();
    chshapes_(1,chunk)=avb.nChannel();
    chshapes_(2,chunk)=avb.nRow();
    chshapes_(3,chunk)=vi.numberAnt();

    for(unsigned int i = 0; i < loadAxes.size(); i++) {
      loadAxis(avb, chunk, loadAxes[i], loadData[i]);
    }
      
    // If a thread is given, update it.
    if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			  chunk % THREAD_SEGMENT == 0)) {
      progress = ((double)chunk+1) / nChunk_;
      thread->setProgress((unsigned int)((progress * 100) + 0.5));
    }
  }
}

void PlotMSCache::forceVBread(VisBuffer& vb,
			      vector<PMS::Axis> loadAxes,
			      vector<PMS::DataColumn> loadData) {

  // pre-load requisite pieces of VisBuffer for averaging
  for(unsigned int i = 0; i < loadAxes.size(); i++) {
    switch (loadAxes[i]) {
    case PMS::AMP: 
    case PMS::PHASE: 
    case PMS::REAL: 
    case PMS::IMAG: {
      switch(loadData[i]) {
      case PMS::DATA: {
	vb.visCube();
	break;
      }
      case PMS::MODEL: {
	vb.modelVisCube();
	break;
      }
      case PMS::CORRECTED: {
	vb.correctedVisCube();
	break;
      }
      case PMS::RESIDUAL: {
	vb.correctedVisCube();
	vb.modelVisCube();
	break;
      }
      default:
	break;
      }
      break;
    }
    default:
      break;
    }
  }
  
  // Always need flags
  vb.flagRow();
  vb.flagCube();

}

void PlotMSCache::discernData(vector<PMS::Axis> loadAxes,
			      vector<PMS::DataColumn> loadData,
			      PlotMSVBAverager& vba) {

  // Turn off 
  vba.setNoData();

  // Tell the averager which data column to read
  for(unsigned int i = 0; i < loadAxes.size(); i++) {
    switch (loadAxes[i]) {
    case PMS::AMP: 
    case PMS::PHASE: 
    case PMS::REAL: 
    case PMS::IMAG: {
      switch(loadData[i]) {
      case PMS::DATA: {
	//	cout << "Arranging to load VC." << endl;
	vba.setDoVC();
	break;
      }
      case PMS::MODEL: {
	//	cout << "Arranging to load MVC." << endl;
	vba.setDoMVC();
	break;
      }
      case PMS::CORRECTED: {
	//	cout << "Arranging to load CVC." << endl;
	vba.setDoCVC();
	break;
      }
      case PMS::RESIDUAL: {
	//	cout << "Arranging to load CVC & MVC." << endl;
	vba.setDoCVC();
	vba.setDoMVC();
	break;
      }
      default:
	break;
      }
      break;
    }
    case PMS::UVDIST:
    case PMS::UVDIST_L:
    case PMS::U:
    case PMS::V:
    case PMS::W: {
      //  cout << "Arranging to load UVW
      vba.setDoUVW();
    }
    default:
      break;
    }
  }

}

void PlotMSCache::setUpVisIter(const String& msname,
			       const PlotMSSelection& selection,
			       Bool readonly,
			       Bool chanselect,
			       Bool corrselect) {

  Bool combscan(averaging_.scan());
  Bool combfld(averaging_.field());
  Bool combspw(averaging_.spw());
  
  Int nsortcol(4+Int(!combscan));  // include room for scan
  Block<Int> columns(nsortcol);
  Int i(0);
  Double iterInterval(0.0);
  if (averaging_.time())
    iterInterval= averaging_.timeValue();

  columns[i++]=MS::ARRAY_ID;
  if (!combscan) columns[i++]=MS::SCAN_NUMBER;  // force scan boundaries
  if (!combfld) columns[i++]=MS::FIELD_ID;      // force field boundaries
  if (!combspw) columns[i++]=MS::DATA_DESC_ID;  // force spw boundaries
  columns[i++]=MS::TIME;
  if (combspw || combfld) iterInterval=DBL_MIN;  // force per-timestamp chunks
  if (combfld) columns[i++]=MS::FIELD_ID;      // effectively ignore field boundaries
  if (combspw) columns[i++]=MS::DATA_DESC_ID;  // effectively ignore spw boundaries
 

  // Now open the MS, select on it, make the VisIter

  Table::TableOption tabopt(Table::Update);
  if (readonly) tabopt=Table::Old;

  MeasurementSet ms(msname,
		    TableLock(TableLock::AutoLocking), tabopt), selms;

  // Apply selection
  Vector<Vector<Slice> > chansel;
  Vector<Vector<Slice> > corrsel;
  selection.apply(ms,selms,chansel,corrsel);

  if (readonly) {
    // Readonly version, for caching
    rvi_p = new ROVisIterator(selms,columns,iterInterval);
    wvi_p =NULL;
  }
  else {
    // Writable, e.g. for flagging
    wvi_p = new VisIterator(selms,columns,iterInterval);
    rvi_p = wvi_p;  // const access
  }

  // Apply chan/corr selction
  if (chanselect) rvi_p->selectChannel(chansel);
  if (corrselect) rvi_p->selectCorrelation(corrsel);

}


      
void PlotMSCache::countChunks(ROVisibilityIterator& vi) {

  // This is the old way, with no averaging over chunks.

  VisBuffer vb(vi);
  
  vi.originChunks();
  vi.origin();
  refTime_p=86400.0*floor(vb.time()(0)/86400.0);

  // Count number of chunks.
  int chunk = 0;
  for(vi.originChunks(); vi.moreChunks(); vi.nextChunk())
    for (vi.origin(); vi.more(); vi++) chunk++;
  if(chunk != nChunk_) increaseChunks(chunk);
  
  //  cout << "Found " << nChunk_ << " " << chunk << " chunks." << endl;

}


void PlotMSCache::countChunks(ROVisibilityIterator& vi, Vector<Int>& nIterPerAve,
			      const PlotMSAveraging& averaging) {

  Bool verby(False);

  Bool combscan(averaging_.scan());
  Bool combfld(averaging_.field());
  Bool combspw(averaging_.spw());

  VisBuffer vb(vi);
  
  vi.originChunks();
  vi.origin();
  
  nIterPerAve.resize(100);
  nIterPerAve=0;
  
  Double time0(86400.0*floor(vb.time()(0)/86400.0));
  refTime_p=time0;
  Double time1(0.0),time(0.0);
  
  Int thisscan(-1),lastscan(-1);
  Int thisfld(-1),lastfld(-1);
  Int thisspw(-1),lastspw(-1);
  Int chunk(0);
  Int ave(-1);
  Double interval(0.0);
  if (averaging.time())
    interval= averaging.timeValue();
  Double avetime1(-1.0);

  vi.originChunks();
  vi.origin();
  stringstream ss;
  
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk(),chunk++) {
    Int iter(0);
    for (vi.origin(); vi.more();vi++,iter++) {

      time1=vb.time()(0);  // first time in this vb
      thisscan=vb.scan()(0);
      thisfld=vb.fieldId();
      thisspw=vb.spectralWindow();

      // New chunk means new ave interval, IF....
      if ( // (!combfld && !combspw) ||                // not combing fld nor spw, OR
	  ((time1-avetime1)>interval) ||         // (combing fld and/or spw) and solint exceeded, OR
	  ((time1-avetime1)<0.0) ||                // a negative time step occurs, OR
	  (!combscan && (thisscan!=lastscan)) ||   // not combing scans, and new scan encountered OR
	  (!combspw && (thisspw!=lastspw)) ||      // not combing spws, and new spw encountered  OR
	  (!combfld && (thisfld!=lastfld)) ||      // not combing fields, and new field encountered OR
	  (ave==-1))  {                            // this is the first interval

	if (verby) {
	  ss << "--------------------------------\n";
	  ss << boolalpha << interval << " " 
	       << ((time1-avetime1)>interval)  << " "
	       << ((time1-avetime1)<0.0) << " "
	       << (!combscan && (thisscan!=lastscan)) << " "
	       << (!combspw && (thisspw!=lastspw)) << " "
	       << (!combfld && (thisfld!=lastfld)) << " "
	       << (ave==-1) << "\n";
	}

	avetime1=time1;  // for next go
	ave++;
	
	if (verby) ss << "ave = " << ave << "\n";


	// increase size of nIterPerAve array, if needed
	if (nIterPerAve.nelements()<uInt(ave+1))
	  nIterPerAve.resize(nIterPerAve.nelements()+100,True);
	nIterPerAve(ave)=0;
      }
      
      // Increment chunk-per-sol count for current solution
      nIterPerAve(ave)++;
      
      if (verby) {
	ss << "          ck=" << chunk << " " << avetime1-time0 << "\n";
	time=vb.time()(0);
	ss  << "                 " << "vb=" << iter << " ";
	ss << "ar=" << vb.arrayId() << " ";
	ss << "sc=" << vb.scan()(0) << " ";
	if (!combfld) ss << "fl=" << vb.fieldId() << " ";
	if (!combspw) ss << "sp=" << vb.spectralWindow() << " ";
	ss << "t=" << floor(time-time0)  << " (" << floor(time-avetime1) << ") ";
	if (combfld) ss << "fl=" << vb.fieldId() << " ";
	if (combspw) ss << "sp=" << vb.spectralWindow() << " ";
	ss << "\n";
	
      }
      
      lastscan=thisscan;
      lastfld=thisfld;
      lastspw=thisspw;
    }
  }
  
  Int nAve(ave+1);
  nIterPerAve.resize(nAve,True);
  
  if (verby)  ss << "nIterPerAve = " << nIterPerAve;
  if (verby) logInfo("count_chunks", ss.str());


  if (nChunk_ != nAve) increaseChunks(nAve);
  
}


#define PMSC_DELETE(VAR)                                                \
  for(unsigned int j = 0; j < VAR.size(); j++)  			\
    if(VAR[j]) delete VAR[j];                                           \
  VAR.resize(0,True);

void PlotMSCache::release(const vector<PMS::Axis>& axes) {
    for(unsigned int i = 0; i < axes.size(); i++) {
        switch(axes[i]) {
        case PMS::SCAN: scan_.resize(0); break;
        case PMS::FIELD: field_.resize(0); break;
        case PMS::TIME: time_.resize(0); break;
        case PMS::TIME_INTERVAL: timeIntr_.resize(0); break;
        case PMS::SPW: spw_.resize(0); break;
        
        case PMS::CHANNEL: PMSC_DELETE(chan_) break;
        case PMS::FREQUENCY: PMSC_DELETE(freq_) break;
        case PMS::VELOCITY: PMSC_DELETE(vel_) break;
        case PMS::CORR: PMSC_DELETE(corr_) break;
        case PMS::ANTENNA1: PMSC_DELETE(antenna1_) break;
        case PMS::ANTENNA2: PMSC_DELETE(antenna2_) break;
        case PMS::BASELINE: PMSC_DELETE(baseline_) break;
        case PMS::UVDIST: PMSC_DELETE(uvdist_) break;
        case PMS::UVDIST_L: PMSC_DELETE(uvdistL_) break;
        case PMS::U: PMSC_DELETE(u_) break;
        case PMS::V: PMSC_DELETE(v_) break;
        case PMS::W: PMSC_DELETE(w_) break;
        case PMS::AMP: PMSC_DELETE(amp_) break;
        case PMS::PHASE: PMSC_DELETE(pha_) break;
        case PMS::REAL: PMSC_DELETE(real_) break;
        case PMS::IMAG: PMSC_DELETE(imag_) break;
        case PMS::FLAG: PMSC_DELETE(flag_) break;
        case PMS::FLAG_ROW: PMSC_DELETE(flagrow_) break;

	case PMS::AZ0: az0_.resize(0); break;
	case PMS::EL0: el0_.resize(0); break;
	case PMS::HA0: ha0_.resize(0); break;
	case PMS::PA0: pa0_.resize(0); break;

        case PMS::ANTENNA: PMSC_DELETE(antenna_) break;
        case PMS::AZIMUTH: PMSC_DELETE(az_) break;
        case PMS::ELEVATION: PMSC_DELETE(el_) break;
        case PMS::PARANG: PMSC_DELETE(parang_) break;
        case PMS::ROW: PMSC_DELETE(row_) break;
	default: break;
        }        

        loadedAxes_[axes[i]] = false;
        
        if(dataLoaded_ && axisIsMetaData(axes[i])) dataLoaded_ = false;
        
        if((dataLoaded_ || axesSet_) &&
           (currentX_ == axes[i] || currentY_ == axes[i])) {
            dataLoaded_ = false;
            axesSet_ = false;
        }
    }
    
    if(!dataLoaded_ || !axesSet_) nChunk_ = 0;
}

void PlotMSCache::setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis) {

  // Put axes on a Vector so we can loop over them below
  Vector<Int> plaxes(2);
  plaxes(0)=xAxis;
  plaxes(1)=yAxis;

  // Set up masks that we use to realize axes relationships
  Matrix<Bool> xymask(4,2);

  xymask.set(False);
  Vector<Bool> xmask(xymask.column(0));
  Vector<Bool> ymask(xymask.column(1));
  getAxesMask(xAxis,xmask);
  getAxesMask(yAxis,ymask);

  Vector<Bool> nmask=(xmask || ymask);
  //  Vector<Bool> cmask=(xmask && ymask);

  netAxesMask_.resize(4);
  netAxesMask_=nmask;

  // cout << boolalpha;
  //  cout << "xmask = " << xmask << endl;
  //  cout << "ymask = " << ymask << endl;
  // cout << "nmask = " << nmask << endl;
  //  cout << "cmask = " << cmask << endl;


  // Forbid antenna-based/baseline-based combination plots, for now
  //  (e.g., data vs. _antenna-based_ elevation)
  if (nmask(2)&&nmask(3))
    throw(AipsError("Cannot yet support antenna-based and baseline-based data in same plot."));
 
  icorrmax_.reference(chshapes_.row(0));
  ichanmax_.reference(chshapes_.row(1));
  ibslnmax_.reference(chshapes_.row(2));
  iantmax_.reference(chshapes_.row(3));
  
  idatamax_.resize(nChunk_);
  idatamax_=chshapes_.row(0);
  idatamax_*=chshapes_.row(1);
  idatamax_*=chshapes_.row(2);
  
  ichanbslnmax_.resize(nChunk_);
  ichanbslnmax_=chshapes_.row(1);
  ichanbslnmax_*=chshapes_.row(2);
  
  nperchan_.resize(nChunk_);
  nperchan_.set(1);
  if (nmask(0)) nperchan_*=chshapes_.row(0);
  
  nperbsln_.resize(nChunk_);
  nperbsln_.set(1);
  if (nmask(0)) nperbsln_*=chshapes_.row(0);
  if (nmask(1)) nperbsln_*=chshapes_.row(1);

  nperant_.reference(nperbsln_);


  for (Int ichk=0;ichk<nChunk_;++ichk) {

    // create a collapsed version of the flags for this chunk
    setPlotMask(ichk);
    
  }


  // Count up the total number of points we will plot
  //   (keep a cumualtive running total)

  Int cumulativeN(0);
  for (Int ic=0;ic<nChunk_;++ic) {
    Int chN=1;
    for (Int ii=0;ii<4;++ii) 
      if (nmask(ii))
	chN*=chshapes_.column(ic)(ii);
    cumulativeN+=chN;
    nPoints_(ic)=cumulativeN;
  }

  //  cout << "nPoints_         = " << nPoints_ << endl;

  //  cout << "nChunk    = " << nChunk() << endl;
  //  cout << "nPoints() = " << nPoints() << endl;


  currentX_ = xAxis; currentY_ = yAxis;

  // Compute the axes ranges
  computeRanges();

  // Plot axes and ranges are now set 
  axesSet_ = true;

}


void PlotMSCache::getAxesMask(PMS::Axis axis,Vector<Bool>& axismask) {

  // Nominally all False
  axismask.set(False);

  switch(axis) {
  case PMS::AMP:
  case PMS::PHASE:
  case PMS::REAL:
  case PMS::IMAG:
  case PMS::FLAG:
    axismask(Slice(0,3,1))=True;
    break;
  case PMS::CHANNEL:
  case PMS::FREQUENCY:
  case PMS::VELOCITY:
    axismask(1)=True;
    break;
  case PMS::CORR:
    axismask(0)=True;
    break;
  case PMS::ROW:
  case PMS::ANTENNA1:
  case PMS::ANTENNA2:
  case PMS::BASELINE:
  case PMS::UVDIST:
  case PMS::U:
  case PMS::V:
  case PMS::W:
  case PMS::FLAG_ROW:
    axismask(2)=True;
    break;
  case PMS::UVDIST_L:
    axismask(1)=True;
    axismask(2)=True;
    break;
  case PMS::ANTENNA:
  case PMS::AZIMUTH:
  case PMS::ELEVATION:
  case PMS::PARANG:
    axismask(3)=True;
    break;
  case PMS::TIME:
  case PMS::TIME_INTERVAL:
  case PMS::SCAN:
  case PMS::SPW:
  case PMS::FIELD:
  case PMS::AZ0:
  case PMS::EL0:
  case PMS::HA0:
  case PMS::PA0:
  default:
    break;
  }

}




Double PlotMSCache::getX(Int i) {

  // Find correct chunk and index offset
  setChunk(i);

  return get(currentX_);
		
}

Double PlotMSCache::getY(Int i) {

  // Find correct chunk and index offset
  setChunk(i);

  return get(currentY_);
  
}

void PlotMSCache::getXY(Int i,Double& x,Double& y) {

  // Find correct chunk and index offset
  setChunk(i);

  x= get(currentX_);
  y= get(currentY_);

}


Bool PlotMSCache::getFlagMask(Int i) {

  // Find correct chunk and index offset
  setChunk(i);
  
  return !(*(plmask_[currChunk_]->data()+irel_));
};

Double PlotMSCache::get(PMS::Axis axis) {

  // Call axis-specific gets
  switch(axis) {
    // Degenerate ones
  case PMS::SCAN:
    return getScan();
    break;
  case PMS::FIELD:
    return getField();
    break;
  case PMS::TIME:
    return getTime();
    break;
  case PMS::TIME_INTERVAL:
    return getTimeIntr();
    break;
  case PMS::SPW:
    return getSpw();
    break;

    // Partial shapes
  case PMS::FREQUENCY:
    return getFreq();
    break;
  case PMS::VELOCITY:
    return getVel();
    break;
  case PMS::CHANNEL:
    return getChan();
    break;
  case PMS::CORR:
    return getCorr();
    break;
  case PMS::ANTENNA1:
    return getAnt1();
    break;
  case PMS::ANTENNA2:
    return getAnt2();
    break;
  case PMS::BASELINE:
    return getBsln();
    break;

  case PMS::UVDIST:
    return getUVDist();
    break;
  case PMS::U:
    return getU();
    break;
  case PMS::V:
    return getV();
    break;
  case PMS::W:
    return getW();
    break;
  case PMS::UVDIST_L:
    return getUVDistL();
    break;

    // Data
  case PMS::AMP:
    return getAmp();
    break;
  case PMS::PHASE:
    return getPha();
    break;
  case PMS::REAL:
    return getReal();
    break;
  case PMS::IMAG:
    return getImag();
    break;
  case PMS::FLAG:
    return getFlag();
    break;

  case PMS::FLAG_ROW:
    return getFlagRow();
    break;

  case PMS::AZ0:
    return getAz0();
    break;
  case PMS::EL0:
    return getEl0();
    break;
  case PMS::HA0:
    return getHA0();
    break;
  case PMS::PA0:
    return getPA0();
    break;

  case PMS::ANTENNA:
    return getAntenna();
    break;
  case PMS::AZIMUTH:
    return getAz();
    break;
  case PMS::ELEVATION:
    return getEl();
    break;
  case PMS::PARANG:
    return getParAng();
    break;
  case PMS::ROW:
    return getRow();
    break;
  default:
    throw(AipsError("Help!"));
  }

}


void PlotMSCache::getRanges(Double& minX, Double& maxX, Double& minY,
			    Double& maxY) {

  // Return what was sent if none available here
  if(!axesSet_) return;
  
  minX = minX_; maxX = maxX_;
  minY = minY_; maxY = maxY_;
}


PlotLogMessage* PlotMSCache::locateNearest(Double x, Double y) {
  
  throw(AipsError("locateNearest is disabled."));

  // WARNING: Hardwired to for amp vs freq plot
  /*
  Double mindist(DBL_MAX);
  Int minic(0),minidx(0);

  Vector<Double> freqdist;
  Double fdist;
  Float adist,dist;

  for (Int ic=0;ic<nChunk_;++ic) {
    Int npts=amp_[ic]->nelements();
    if (npts>0) {

      freqdist.resize(0);
      freqdist=(*freq_[ic]);
      freqdist-=x;

      Float *a = amp_[ic]->data();
      for (Int i=0;i<npts;++i,++a) {
	adist= *a - Float(y);
	adist/=Float(maxY_-minY_);  // Unitless!
	adist*=adist;
	
	//	fdist=*(freq_[ic]->data()+(i/xdiv_)%xmod_) - Double(x);
	fdist=freqdist((i/xdiv_)%xmod_);
	fdist/=(maxX_-minX_);       // Unitless!
	fdist*=fdist;

	dist=adist+Float(fdist);

	if (dist<mindist) {
	  minic=ic;
	  minidx=i;
	  mindist=dist;
	}
      }

    }
  }

  stringstream ss;
  reportMeta(minic,minidx,ss);
  return new PlotLogGeneric(PlotMS::CLASS_NAME, PlotMS::LOG_LOCATE, ss.str());

  */

}


PlotLogMessage* PlotMSCache::locateRange(const Vector<PlotRegion>& regions) {    
    Timer locatetimer;
    locatetimer.mark();
    
    Double thisx, thisy;
    stringstream ss;
    Int nFound = 0, n = nPoints();
    
    for(Int i = 0; i < n; i++) {

      // Only report unflagged data for now
      //  TBD: report unflagged data if they are visible!
      if (!getFlagMask(i)) {

        getXY(i, thisx, thisy);
        
        for(uInt j = 0; j < regions.size(); j++) {
	  if(thisx > regions[j].left() && thisx < regions[j].right() &&
	     thisy > regions[j].bottom() && thisy < regions[j].top()) {
	    nFound++;
	    reportMeta(thisx, thisy, ss);
	    ss << '\n';
	    break;
	  }
        }
      }
    }
    
    ss << "Found " << nFound << " points among " << n << " in "
       << locatetimer.all_usec()/1.0e6 << "s.";
    
    return new PlotLogMessage(PMS::LOG_ORIGIN,PMS::LOG_ORIGIN_LOCATE,ss.str(),PMS::LOG_EVENT_LOCATE);
}


PlotLogMessage* PlotMSCache::flagRange(const PlotMSFlagging& flagging,
				       const Vector<PlotRegion>& regions, Bool flag) {
    Timer flagtimer;
    flagtimer.mark();

    // List of flags
    Vector<Int> flagchunk(1000,-1),flagindex(1000,-1);

    Double thisx, thisy;
    stringstream ss;
    Int nFound = 0, n = nPoints(), flsz;
    
    for(Int i = 0; i < n; i++) {      
        getXY(i, thisx, thisy);
        
        for(uInt j = 0; j < regions.size(); j++) {
            if(thisx > regions[j].left() && thisx < regions[j].right() &&
               thisy > regions[j].bottom() && thisy < regions[j].top()) {
                nFound++;
                
                flagInCache(flagging, flag);
                
                // Record this flags indices so we can apply to MS below
                flsz = flagchunk.nelements();
                if(flsz < nFound) {
                    // Add 50% more space (w/ copy!)
                    flagchunk.resize(Int(flsz * 1.5), True);
                    flagindex.resize(Int(flsz * 1.5), True);
                }
                flagchunk(nFound - 1) = currChunk_;
                flagindex(nFound - 1) = irel_;
                
                //  reportMeta(thisx,thisy,ss);
                //  ss << '\n';
            }
        }
    }

    // Apply flags only if some found
    if (nFound>0) {
    
      // Refresh the plot mask to reflect newly flagged data
      for(Int ichk = 0; ichk < nChunk_; ichk++)
        setPlotMask(ichk);
      
      // shrink flag list to correct size
      if(flagchunk.nelements() > uInt(nFound)) {
        flagchunk.resize(nFound, True);
        flagindex.resize(nFound, True);
      }
      
      // Set the flags in the MS
      flagInMS(flagging, flagchunk, flagindex, flag);


      // Re-compute the axes ranges
      computeRanges();
    }

    ss << (flag ? "FLAGGED " : "UNFLAGGED ") << nFound 
       << " points among " << n << " in "
       << flagtimer.all_usec()/1.0e6 << "s.";

    return new PlotLogMessage(PMS::LOG_ORIGIN,
            flag ? PMS::LOG_ORIGIN_FLAG : PMS::LOG_ORIGIN_UNFLAG, ss.str(),
            flag ? PMS::LOG_EVENT_FLAG : PMS::LOG_EVENT_UNFLAG);
}


void PlotMSCache::flagInCache(const PlotMSFlagging& flagging,Bool flag) {

  Slice corr,chan,bsln;

  // Set flag range on correlation axis:
  Int icorr(-1);
  if (netAxesMask_(0) && !flagging.corrAll()) {
    // specific correlation
    icorr=(irel_%icorrmax_(currChunk_));
    corr=Slice(icorr,1,1);
  } 
  else 
    // All correlations
    corr=Slice(0,chshapes_(0,currChunk_),1);
   
  // Set Flag range on channel axis:
  Int ichan(-1);
  if (netAxesMask_(1) && !flagging.channel()) {
    // specific channel
    ichan=Int(getChan());
    if (averaging_.channel()) {
      // correct to the _in-cache_ channel _index_ 
      Int dch=Int(averaging_.channelValue());
      if (dch>1) {
	ichan/=dch;
	ichan-=((*chan_[currChunk_])(0)/dch);
      }
    }
    else
      // when not averaging, maybe first channel isn't 0th
      ichan-=(*chan_[currChunk_])(0);  

    chan=Slice(ichan,1,1);
  } 
  else 
    // All channels
    chan=Slice(0,chshapes_(1,currChunk_),1);
  

  // Set Flag range on baseline axis:
  Int ibsln(-1);
  if (netAxesMask_(2)) {
    // specific correlation
    ibsln=(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_);
    bsln=Slice(ibsln,1,1);
  } 
  else 
    // All baselines
    bsln=Slice(0,chshapes_(2,currChunk_),1);
  
  // Set the sliced flag
  Cube<Bool> flagcube(*flag_[currChunk_]);
  flagcube(corr,chan,bsln)=flag;

}

void PlotMSCache::setPlotMask(Int chunk) {

  IPosition nsh(3,1,1,1),csh;
  
  for (Int iax=0;iax<3;++iax) {
    if (netAxesMask_(iax)) 
      // non-trivial size for this axis
      nsh(iax)=chshapes_(iax,chunk);
    else 
      // add this axis to collapse list
      csh.append(IPosition(1,iax));
  }
  
  
  if (netAxesMask_(3) && !netAxesMask_(2)) {
    nsh(2)=iantmax_(chunk);
    plmask_[chunk]->resize(nsh);
    plmask_[chunk]->set(True);
  }
  else {
    plmask_[chunk]->resize(nsh);
    (*plmask_[chunk]) = operator>(partialNFalse(*flag_[chunk],csh).reform(nsh),uInt(0));
  }
  
}

void PlotMSCache::flagInMS(const PlotMSFlagging& flagging,
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

  // Make the VisIterator writable, with selection revised as appropriate
  Bool selectchan(netAxesMask_(1) && !flagging.channel());
  Bool selectcorr(netAxesMask_(0) && !flagging.corrAll());

  // Establish a scope in which the VisBuffer is properly created/destroyed
  {
  setUpVisIter(msname_,selection_,False,selectchan,selectcorr);
  VisBuffer vb(*wvi_p);

  wvi_p->originChunks();
  wvi_p->origin();

  Int iflag(0);
  for (Int ichk=0;ichk<nChunk_;++ichk) {

    if (ichk!=flchunks(order[iflag])) {
      // Step over current chunk
      for (Int i=0;i<nVBPerAve_(ichk);++i) {
	wvi_p->operator++();
	if (!wvi_p->more() && wvi_p->moreChunks()) {
	  wvi_p->nextChunk();
	  wvi_p->origin();
	}
      }
    }
    else {

      // This chunk requires flag-setting

      // For each VB in this cache chunk
      Int ifl(iflag);
      for (Int i=0;i<nVBPerAve_(ichk);++i) {

	// Refer to VB pieces we need
	Cube<Bool> vbflag(vb.flagCube());
	Vector<Int> corrType(vb.corrType());
	Vector<Int> channel(vb.channel());
	Vector<Int> a1(vb.antenna1());
	Vector<Int> a2(vb.antenna2());
	Int ncorr=corrType.nelements();
	Int nchan=channel.nelements();
	Int nrow=vb.nRow();
	if (False) {
	  currChunk_=flchunks(order[iflag]);
	  irel_=flrelids(order[iflag]);
	  ss << "Time diff: " << getTime()-vb.time()(0) << " " << getTime() << " " << vb.time()(0) << "\n";
	  ss << "Spw diff:  " << Int(getSpw())-vb.spectralWindow() << " " << getSpw() << " " << vb.spectralWindow() << "\n";
	  ss << "Field diff:  " << Int(getField())-vb.fieldId() << " " << getField() << " " << vb.fieldId() << "\n";
	}

	// Apply all flags in this chunk to this VB
	ifl=iflag;
	while (ifl<Int(nflag) && flchunks(order[ifl])==ichk) {

	  currChunk_=flchunks(order[ifl]);
	  irel_=flrelids(order[ifl]);

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
	    Int ichan=(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_);
	    if (averaging_.channel() && averaging_.channelValue()>0) {
	      Int start=chanAveBounds_p(vb.spectralWindow())(ichan,2);
	      Int n=chanAveBounds_p(vb.spectralWindow())(ichan,3)-start+1;
	      chan=Slice(start,n,1);
	    }
	    else
	      // A single specific channel 
	      chan=Slice(ichan,1,1);
	  }
	  else 
	    // Extend to all channels
	    chan=Slice(0,nchan,1);
	  
	  // Set Flags on the baseline axis:
	  Int thisA1=Int(getAnt1()), thisA2=Int(getAnt2());
	  if (netAxesMask_(2) && 
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
		  if (False) {
		    cout << i << " " << ifl << " " << irow << " " << a1(irow) << "-" << a2(irow) 
			 << " corr: " << corr.start() << " " << corr.length()
			 << " chan: " << chan.start() << " " << chan.length()
			 << endl;
		  }
		  vbflag(corr,chan,Slice(irow,1,1))=flag;

		  break;  // found the one baseline, escape from for loop
		}
	      }
	      else {
		// either antenna matches the one specified antenna
		//  (don't break because there will be more than one)
		if (a1(irow)==thisA1 ||
		    a2(irow)==thisA1) 
		  vbflag(corr,chan,Slice(irow,1,1))=flag;
	      }
	    }
	  }
	  else {
	    // Set flags for all baselines, because the plot
	    //  is ordinarily implicit in baseline, we've turned on baseline
	    //  extension, or we've avaraged over all baselines
	    bsln=Slice(0,nrow,1);
	    vbflag(corr,chan,bsln)=flag;
	  } 
	  
	  ++ifl;
	}

	// Put the flags back into the MS
	wvi_p->setFlag(vbflag);

	// Advance to the next vb
	wvi_p->operator++();
	if (!wvi_p->more() && wvi_p->moreChunks()) {
	  wvi_p->nextChunk();
	  wvi_p->origin();
	}
      }  // VBs in this averaging chunk

      // step over the flags we've just done
      iflag=ifl;

      // Escape if we are already finished
      if (uInt(iflag)>=nflag) break;

    } // flagable VB
    

  } // ichk

  // Close the scope that holds the VisBuffer used above
  }

  // Delete the VisIter so lock is released
  if (wvi_p)
    delete wvi_p;
  wvi_p=NULL;
  rvi_p=NULL;

  logFlag(ss.str());

}

vector<pair<PMS::Axis, unsigned int> > PlotMSCache::loadedAxes() const {    
    // have to const-cast loaded axes because the [] operator is not const,
    // even though we're not changing it.
    map<PMS::Axis, bool>& la = const_cast<map<PMS::Axis, bool>& >(loadedAxes_);
    
    vector<pair<PMS::Axis, unsigned int> > v;
    const vector<PMS::Axis>& axes = PMS::axes();
    for(unsigned int i = 0; i < axes.size(); i++)
        if(la[axes[i]])
            v.push_back(pair<PMS::Axis, unsigned int>(
                        axes[i], nPointsForAxis(axes[i])));
    
    return v;
}

void PlotMSCache::reportMeta(Double x, Double y,stringstream& ss) {

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

}

void PlotMSCache::setChunk(Int i) {

  if (i==0) currChunk_=0;  // probably insufficient as a general reset!

  // Bump up if needed
  if (i > (nPoints_(currChunk_)-1)) ++currChunk_; 

  irel_=i;
  if (currChunk_>0) 
    irel_-=nPoints_(currChunk_-1);

}

void PlotMSCache::deleteCache() {
    // Release all axes.
    release(PMS::axes());
}

void PlotMSCache::loadAxis(VisBuffer& vb, Int vbnum, PMS::Axis axis,
			   PMS::DataColumn data) {    

    switch(axis) {

    case PMS::SCAN: // assumes scan unique in VB
        scan_(vbnum) = vb.scan()(0); 
	break;
        
    case PMS::FIELD:
        field_(vbnum) = vb.fieldId();
	break;
        
    case PMS::TIME: // assumes time unique in VB
        time_(vbnum) = vb.time()(0); 
	break;
        
    case PMS::TIME_INTERVAL: // assumes timeInterval unique in VB
        timeIntr_(vbnum) = vb.timeInterval()(0); 
	break;
        
    case PMS::SPW:
        spw_(vbnum) = vb.spectralWindow(); 
	break;

    case PMS::CHANNEL:
        *chan_[vbnum] = vb.channel(); 
	break;

    case PMS::FREQUENCY: {
      // Convert freq to desired frame
      //      cout << "Loading FREQUENCY" << endl;
      vbu_.convertFrequency(*freq_[vbnum],vb,transformations_.frame());
      (*freq_[vbnum])/=1.0e9; // in GHz
      break;
    }
    case PMS::VELOCITY: {
      // Convert freq in the vb to velocity
      vbu_.toVelocity(*vel_[vbnum],
		      vb,
		      transformations_.frame(),
		      MVFrequency(transformations_.restFreqHz()),
		      transformations_.veldef());
      (*vel_[vbnum])/=1.0e3;  // in km/s
      break;
    }
    case PMS::CORR:
      *corr_[vbnum] = vb.corrType();
      break;
        
    case PMS::ANTENNA1:
        *antenna1_[vbnum] = vb.antenna1(); 
	break;
    case PMS::ANTENNA2:
        *antenna2_[vbnum] = vb.antenna2(); 
	break;
    case PMS::BASELINE: {
      Vector<Int> a1(vb.antenna1());
      Vector<Int> a2(vb.antenna2());
      baseline_[vbnum]->resize(vb.nRow());
      Vector<Int> bl(*baseline_[vbnum]);
      for (Int irow=0;irow<vb.nRow();++irow)
	bl(irow)=chshapes_(3,0) * a1(irow) - (a1(irow) * (a1(irow) - 1)) / 2 + a2(irow) - a1(irow);
	//	bl(irow)=40 * a1(irow) - (a1(irow) * (a1(irow) - 1)) / 2 + a2(irow) - a1(irow);
      //	bl(irow)=40 * a2(irow) - (a2(irow) * (a2(irow) - 1)) / 2 + a1(irow) - a2(irow);
      break;
    }
    case PMS::UVDIST: {
      Vector<Double> u(vb.uvwMat().row(0));
      Vector<Double> v(vb.uvwMat().row(1));
      *uvdist_[vbnum] = sqrt(u*u+v*v);
      break;
    }
    case PMS::U:
      *u_[vbnum] = vb.uvwMat().row(0);
      break;
    case PMS::V:
      *v_[vbnum] = vb.uvwMat().row(1);
      break;
    case PMS::W:
      *w_[vbnum] = vb.uvwMat().row(2);
      break;
    case PMS::UVDIST_L: {
      Vector<Double> u(vb.uvwMat().row(0));
      Vector<Double> v(vb.uvwMat().row(1));
      Vector<Double> uvdistM = sqrt(u*u+v*v);
      uvdistM /=C::c;
      uvdistL_[vbnum]->resize(vb.nChannel(),vb.nRow());
      Vector<Double> uvrow;
      for (Int irow=0;irow<vb.nRow();++irow) {
	uvrow.reference(uvdistL_[vbnum]->column(irow));
	uvrow.set(uvdistM(irow));
	uvrow*=vb.frequency();
      }
      break;
    }

    case PMS::AMP: {
      switch(data) {
      case PMS::DATA: {

	*amp_[vbnum] = amplitude(vb.visCube());

	// TEST fft on freq axis to get delay
	if (False) {

	  // Only transform frequency axis
	  //   (Should avoid cross-hand data, too?)
	  Vector<Bool> ax(3,False);
	  ax(1)=True;
	  
	  // Support padding for higher delay resolution
	  Int fact(4);
	  IPosition ip=vb.visCube().shape();
	  Int nch=ip(1);
	  ip(1)*=fact;
	  
	  Slicer sl(Slice(),Slice(nch*(fact-1)/2,nch,1),Slice());
	  
	  Array<Complex> vpad(ip);
	  vpad.set(Complex(0.0));
	  vpad(sl)=vb.visCube();
	  
	  
	  cout << "vpad.shape() = " << vpad.shape() << endl;
	  cout << "vpad(sl).shape() = " << vpad(sl).shape() << endl;

	  Vector<Complex> testf(64,Complex(1.0));
	  FFTServer<Float,Complex> ffts;
	  cout << "FFTServer..." << flush;
	  ffts.fft(testf,True);
	  cout << "done." << endl;

	  ArrayLattice<Complex> tf(testf);
	  cout << "tf.isWritable() = " << boolalpha << tf.isWritable() << endl;

	  LatticeFFT::cfft(tf,False);
	  cout << "testf = " << testf << endl;


	  cout << "Starting ffts..." << flush;
	  
	  ArrayLattice<Complex> c(vpad);
	  cout << "c.shape() = " << c.shape() << endl;
	  //	  LatticeFFT::cfft(c,ax);	 
	  LatticeFFT::cfft2d(c,False);	 
	  
	  cout << "done." << endl;
	  
	  *amp_[vbnum] = amplitude(vpad(sl));
	}
	break;

      }
      case PMS::MODEL: {
	*amp_[vbnum] = amplitude(vb.modelVisCube());
	break;
      }
      case PMS::CORRECTED: {
	*amp_[vbnum] = amplitude(vb.correctedVisCube());
	break;
      }
      case PMS::RESIDUAL: {
	*amp_[vbnum] = amplitude(vb.correctedVisCube()-vb.modelVisCube());
	break;
      }
      }
      break;
    }
    case PMS::PHASE: {
      switch(data) {
      case PMS::DATA: {
	*pha_[vbnum] = phase(vb.visCube())*(180.0/C::pi);
	break;
      }
      case PMS::MODEL: {
	*pha_[vbnum] = phase(vb.modelVisCube())*(180.0/C::pi);
	break;
      }
      case PMS::CORRECTED: {
	*pha_[vbnum] = phase(vb.correctedVisCube())*(180.0/C::pi);
	break;
      }
      case PMS::RESIDUAL: {
	*pha_[vbnum] = phase(vb.correctedVisCube()-vb.modelVisCube())*(180.0/C::pi);
	break;
      }
      }
      break;
    }
    case PMS::REAL: {
      switch(data) {
      case PMS::DATA: {
	*real_[vbnum] = real(vb.visCube());
	break;
      }
      case PMS::MODEL: {
	*real_[vbnum] = real(vb.modelVisCube());
	break;
      }
      case PMS::CORRECTED: {
	*real_[vbnum] = real(vb.correctedVisCube());
	break;
      }
      case PMS::RESIDUAL: {
	*real_[vbnum] = real(vb.correctedVisCube())-real(vb.modelVisCube());
	break;
      }
      }
      break;
    }
    case PMS::IMAG: {
      switch(data) {
      case PMS::DATA: {
	*imag_[vbnum] = imag(vb.visCube());
	break;
      }
      case PMS::MODEL: {
	*imag_[vbnum] = imag(vb.modelVisCube());
	break;
      }
      case PMS::CORRECTED: {
	*imag_[vbnum] = imag(vb.correctedVisCube());
	break;
      }
      case PMS::RESIDUAL: {
	*imag_[vbnum] = imag(vb.correctedVisCube())-imag(vb.modelVisCube());
	break;
      }
      }
      break;
    }
                
    case PMS::FLAG:
      *flag_[vbnum] = vb.flagCube();
      break;
    case PMS::FLAG_ROW:
      *flagrow_[vbnum] = vb.flagRow();
      break;

    case PMS::AZ0:
    case PMS::EL0: {
      Vector<Double> azel;
      vb.azel0Vec(vb.time()(0),azel);
      az0_(vbnum) = azel(0);
      el0_(vbnum) = azel(1);
      break;
    }
    case PMS::HA0: 
      ha0_(vbnum) = vb.hourang(vb.time()(0))*12/C::pi;  // in hours
      break;
    case PMS::PA0:
      pa0_(vbnum) = vb.parang0(vb.time()(0))*180.0/C::pi; // in degrees
      break;

    case PMS::ANTENNA: {
      antenna_[vbnum]->resize(nAnt_);
      indgen(*antenna_[vbnum]);
      break;
    }
    case PMS::AZIMUTH:
    case PMS::ELEVATION: {
      Matrix<Double> azel;
      vb.azelMat(vb.time()(0),azel);
      *az_[vbnum] = azel.row(0);
      *el_[vbnum] = azel.row(1);
      break;
    }
    case PMS::PARANG:
      *parang_[vbnum] = vb.feed_pa(vb.time()(0))*(180.0/C::pi);  // in degrees
      break;

    case PMS::ROW:
      *row_[vbnum] = vb.rowIds();
      break;

      /*        
    case PMS::WEIGHT:
      */

    default:
      throw(AipsError("Axis choice NYI"));
      break;
    }
}

unsigned int PlotMSCache::nPointsForAxis(PMS::Axis axis) const {
    switch(axis) {    
    case PMS::FREQUENCY: 
    case PMS::VELOCITY: 
    case PMS::CHANNEL: 
    case PMS::CORR: 
    case PMS::AMP: 
    case PMS::PHASE: 
    case PMS::ANTENNA1:
    case PMS::ANTENNA2: 
    case PMS::BASELINE: 
    case PMS::UVDIST:
    case PMS::UVDIST_L:
    case PMS::U:
    case PMS::V:
    case PMS::W:
    case PMS::FLAG:
    case PMS::ANTENNA: 
    case PMS::AZIMUTH: 
    case PMS::ELEVATION: 
    case PMS::PARANG: 
    case PMS::ROW:
    case PMS::FLAG_ROW: 
      {

        unsigned int n = 0;
        for(unsigned int i = 0; i < freq_.size(); i++) {
            if(axis == PMS::FREQUENCY)     n += freq_[i]->size();
            else if(axis == PMS::VELOCITY) n += vel_[i]->size();
            else if(axis == PMS::CHANNEL)  n += chan_[i]->size();
            else if(axis == PMS::CORR)     n += corr_[i]->size();
            else if(axis == PMS::AMP)      n += amp_[i]->size();
            else if(axis == PMS::PHASE)    n += pha_[i]->size();
            else if(axis == PMS::ROW)      n += row_[i]->size();
            else if(axis == PMS::ANTENNA1) n += antenna1_[i]->size();
            else if(axis == PMS::ANTENNA2) n += antenna2_[i]->size();
            else if(axis == PMS::BASELINE) n += antenna2_[i]->size();
            else if(axis == PMS::UVDIST)   n += uvdist_[i]->size();
            else if(axis == PMS::UVDIST_L) n += uvdistL_[i]->size();
            else if(axis == PMS::U)        n += u_[i]->size();
            else if(axis == PMS::V)        n += v_[i]->size();
            else if(axis == PMS::W)        n += w_[i]->size();
            else if(axis == PMS::FLAG)     n += flag_[i]->size();
            else if(axis == PMS::ANTENNA)  n += antenna_[i]->size();
            else if(axis == PMS::AZIMUTH)  n += az_[i]->size();
            else if(axis == PMS::ELEVATION)n += el_[i]->size();
            else if(axis == PMS::PARANG)   n += parang_[i]->size();
	    else if(axis == PMS::FLAG_ROW) n += flagrow_[i]->size();
        }
        return n;
    }     
    
    case PMS::TIME:          return time_.size();
    case PMS::TIME_INTERVAL: return timeIntr_.size();    
    case PMS::FIELD:         return field_.size();      
    case PMS::SCAN:          return scan_.size();     
    case PMS::SPW:           return spw_.size();     
        
    case PMS::AZ0:           return az0_.size();
    case PMS::EL0:           return el0_.size();
    case PMS::HA0:           return ha0_.size();
    case PMS::PA0:           return pa0_.size();

    default: return 0;
    }
}

void PlotMSCache::computeRanges() {

  stringstream ss;
  ss << "Computing ranges...";

  nTotalPoints_=0;
  nFlagPoints_=0;
  nUnFlagPoints_=0;

  Vector<Int> plaxes(2);
  plaxes(0)=currentX_;
  plaxes(1)=currentY_;
  

  Vector<Double> limits(4);
  limits(0)=limits(2)=DBL_MAX;
  limits(1)=limits(3)=-DBL_MAX;
  
  Int totalN(0);
  for (Int ic=0;ic<nChunk_;++ic) {
    
    nTotalPoints_+=plmask_[ic]->nelements();
    
    Int thisN=ntrue(*plmask_[ic]);
    nUnFlagPoints_+=thisN;
    if (thisN >0) {
      totalN+=thisN;
      for (Int ix=0;ix<2;++ix) {
	
	// Arrange collapsed-on-axis mask
	Array<Bool> collmask;
	switch(plaxes(ix)) {
	case PMS::FREQUENCY: 
	case PMS::VELOCITY: 
	case PMS::CHANNEL: {
	  // collapse on corr, row
	  collmask=operator>(partialNTrue(*plmask_[ic],IPosition(2,0,2)),uInt(0));
	  break;
	}
	case PMS::CORR: {
	  // collapse on chan, row
	  collmask=operator>(partialNTrue(*plmask_[ic],IPosition(2,1,2)),uInt(0));
	  break;
	}
	case PMS::ROW:
	case PMS::ANTENNA1:
	case PMS::ANTENNA2:
	case PMS::BASELINE:
	case PMS::UVDIST:
	case PMS::U:
	case PMS::V:
	case PMS::W: {
	  // collapse on corr,chan
	  collmask=operator>(partialNTrue(*plmask_[ic],IPosition(2,0,1)),uInt(0));
	  break;
	}
	case PMS::UVDIST_L: {
	  // collapse on corr
	  collmask=operator>(partialNTrue(*plmask_[ic],IPosition(1,0)),uInt(0));
	  break;
	}
	case PMS::AMP: 
	case PMS::PHASE:
	case PMS::REAL:
	case PMS::IMAG:{
	  // reference plmask_
	  collmask.reference(*plmask_[ic]);
	  break;
	}
	case PMS::ANTENNA:
	case PMS::AZIMUTH:
	case PMS::ELEVATION:
	case PMS::PARANG:
	  // TBD
	  break;
	default:
	  break;
	}
	
	// Now calculate masked limits
	
	if (True) {
	  
	  switch(plaxes(ix)) {
	  case PMS::SCAN:
	    limits(2*ix)=min(limits(2*ix),Double(scan_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(scan_(ic)));
	    break;
	  case PMS::FIELD:
	    limits(2*ix)=min(limits(2*ix),Double(field_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(field_(ic)));
	    break;
	  case PMS::TIME:
	    limits(2*ix)=min(limits(2*ix),time_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),time_(ic));
	    break;
	  case PMS::TIME_INTERVAL:
	    limits(2*ix)=min(limits(2*ix),timeIntr_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),timeIntr_(ic));
	    break;
	  case PMS::SPW:
	    limits(2*ix)=min(limits(2*ix),Double(spw_(ic)));
	    limits(2*ix+1)=max(limits(2*ix+1),Double(spw_(ic)));
	    break;
	  case PMS::FREQUENCY:
	    if (freq_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*freq_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*freq_[ic])(collmask)));
	    }
	    break;
	  case PMS::VELOCITY:
	    if (vel_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*vel_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*vel_[ic])(collmask)));
	    }
	    break;
	  case PMS::CHANNEL:
	    if (chan_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*chan_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*chan_[ic])(collmask))));
	    }
	    break;
	  case PMS::CORR:
	    if (corr_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*corr_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*corr_[ic])(collmask))));
	    }
	    break;
	  case PMS::ROW:
	    if (row_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*row_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*row_[ic])(collmask))));
	    }
	    break;
	  case PMS::ANTENNA1:
	    if (antenna1_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*antenna1_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*antenna1_[ic])(collmask))));
	    }
	    break;
	  case PMS::ANTENNA2:
	    if (antenna2_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*antenna2_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*antenna2_[ic])(collmask))));
	    }
	    break;
	  case PMS::BASELINE:
	    if (baseline_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*baseline_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*baseline_[ic])(collmask))));
	    }
	    break;
	    
	  case PMS::UVDIST:
	    if (uvdist_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*uvdist_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*uvdist_[ic])(collmask))));
	    }
	    break;
	  case PMS::U:
	    if (u_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*u_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*u_[ic])(collmask))));
	    }
	    break;
	  case PMS::V:
	    if (v_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*v_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*v_[ic])(collmask))));
	    }
	    break;
	  case PMS::W:
	    if (w_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*w_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*w_[ic])(collmask))));
	    }
	    break;
	  case PMS::UVDIST_L:
	    if (uvdistL_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min((*uvdistL_[ic])(collmask))));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max((*uvdistL_[ic])(collmask))));
	    }
	    break;
	    
	  case PMS::AMP: {
	    if (amp_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*amp_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*amp_[ic])(collmask)));
	    }
	    break;
	  }
	  case PMS::PHASE:
	    if (pha_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*pha_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*pha_[ic])(collmask)));
	    }
	    break;
	  case PMS::REAL:
	    if (real_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*real_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*real_[ic])(collmask)));
	    }
	    break;
	  case PMS::IMAG:
	    if (imag_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min((*imag_[ic])(collmask)));
	      limits(2*ix+1)=max(limits(2*ix+1),max((*imag_[ic])(collmask)));
	    }
	    break;
	  case PMS::FLAG:
	  case PMS::FLAG_ROW:
	    if (flag_[ic]->nelements()>0) {
	      limits(2*ix)=0.0;
	      limits(2*ix+1)=1.0;
	    }
	    break;

	  case PMS::AZ0:
	    limits(2*ix)=min(limits(2*ix),az0_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),az0_(ic));
	    break;
	  case PMS::EL0:
	    limits(2*ix)=min(limits(2*ix),el0_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),el0_(ic));
	    break;
	  case PMS::HA0:
	    limits(2*ix)=min(limits(2*ix),ha0_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),ha0_(ic));
	    break;
	  case PMS::PA0:
	    limits(2*ix)=min(limits(2*ix),pa0_(ic));
	    limits(2*ix+1)=max(limits(2*ix+1),pa0_(ic));
	    break;

	  case PMS::ANTENNA:
	    if (antenna_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),Double(min(*antenna_[ic])));
	      limits(2*ix+1)=max(limits(2*ix+1),Double(max(*antenna_[ic])));
	    }
	    break;
	  case PMS::AZIMUTH:
	    if (az_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*az_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*az_[ic]));
	    }
	    break;
	  case PMS::ELEVATION:
	    if (el_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*el_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*el_[ic]));
	    }
	    break;
	  case PMS::PARANG:
	    if (parang_[ic]->nelements()>0) {
	      limits(2*ix)=min(limits(2*ix),min(*parang_[ic]));
	      limits(2*ix+1)=max(limits(2*ix+1),max(*parang_[ic]));
	    }
	    break;
	  default:
	    throw(AipsError("Help PlotMSCache::computeRanges"));
	  }


	}
      }
    }

  }

  nFlagPoints_=nTotalPoints_-nUnFlagPoints_;

  //  cout << nUnFlagPoints_ << " " << nFlagPoints_ << " " << nTotalPoints_ << " (=" << nPoints_(nChunk_-1) << ")" << endl;

  minX_=limits(0);
  maxX_=limits(1);
  minY_=limits(2);
  maxY_=limits(3);
  
  // Pad genericially
  Double dX=0.05*abs(maxX_-minX_);
  Double dY=0.05*abs(maxY_-minY_);
  minX_-=dX;
  maxX_+=dX;
  minY_-=dY;
  maxY_+=dY;

  ss << ": dX=" << minX_ << "-" << maxX_ << " dY=" << minY_ << "-" << maxY_ << "\n";
  ss << "Npoints = " << totalN;
  logInfo("compute_ranges", ss.str());
}

void PlotMSCache::log(const String& method, const String& message,
        int eventType) {
    plotms_->getLogger()->postMessage(PMS::LOG_ORIGIN,method,message,eventType);}

}

//# PlotMSCache2.cc: Data cache for plotms.
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
#include <plotms/Data/PlotMSCache2.h>
#include <plotms/Data/PlotMSIndexer.h>

#include <casa/OS/Timer.h>
#include <casa/OS/HostInfo.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/Utilities/Sort.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <scimath/Mathematics/FFTServer.h>
#include <ms/MeasurementSets/MSColumns.h> 	 
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <plotms/Data/PlotMSVBAverager.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>

namespace casa {

const PMS::Axis PlotMSCache2::METADATA[] =
    { PMS::TIME, PMS::TIME_INTERVAL, PMS::FIELD, PMS::SPW, PMS::SCAN,
      PMS::ANTENNA1, PMS::ANTENNA2, PMS::BASELINE, 
      PMS::CHANNEL, PMS::CORR, PMS::FREQUENCY,
      PMS::FLAG, PMS::FLAG_ROW };
const unsigned int PlotMSCache2::N_METADATA = 13;

bool PlotMSCache2::axisIsMetaData(PMS::Axis axis) {
    for(unsigned int i = 0; i < N_METADATA; i++)
        if(METADATA[i] == axis) return true;
    return false;
}

const unsigned int PlotMSCache2::THREAD_SEGMENT = 10;


PMSCacheVolMeter::PMSCacheVolMeter():
  nDDID_(0),
  nPerDDID_(),
  nRowsPerDDID_(),
  nChanPerDDID_(),
  nCorrPerDDID_(),
  nAnt_(0) {}


PMSCacheVolMeter::PMSCacheVolMeter(const MeasurementSet& ms, const PlotMSAveraging ave):
  nDDID_(0),
  nPerDDID_(),
  nRowsPerDDID_(),
  nChanPerDDID_(),
  nCorrPerDDID_(),
  nAnt_(0) {

  ROMSColumns msCol(ms);

  // Initialize chunks and rows counters
  nDDID_=msCol.dataDescription().nrow();
  nPerDDID_.resize(nDDID_);
  nPerDDID_.set(0);
  nRowsPerDDID_.resize(nDDID_);
  nRowsPerDDID_.set(0);
  nChanPerDDID_.resize(nDDID_);
  nCorrPerDDID_.resize(nDDID_);

  // Fill Corr/Chan-per-DDID Vectors
  Vector<Int> nChanPerSpw;
  msCol.spectralWindow().numChan().getColumn(nChanPerSpw);
  Vector<Int> nCorrPerPol;
  msCol.polarization().numCorr().getColumn(nCorrPerPol);
  Vector<Int> polPerDDID;
  msCol.dataDescription().polarizationId().getColumn(polPerDDID);
  Vector<Int> spwPerDDID;
  msCol.dataDescription().spectralWindowId().getColumn(spwPerDDID);

  Bool chave=(ave.channel() && ave.channelValue()>1.0);

  for (Int iddid=0;iddid<nDDID_;++iddid) {
    // ncorr is simple (for now, maybe Stokes later?):
    nCorrPerDDID_(iddid)=nCorrPerPol(polPerDDID(iddid));
    // nChan depends on averaging:
    Int nchan0=nChanPerSpw(spwPerDDID(iddid));
    Int nchanA=Int(ceil(Double(nchan0)/ave.channelValue()));
    nChanPerDDID_(iddid)= (chave ? nchanA : nchan0);
  }
  //  cout << "nChanPerDDID_ = " << nChanPerDDID_ << endl;
  //  cout << "nCorrPerDDID_ = " << nCorrPerDDID_ << endl;
  // nAnt:
  nAnt_=msCol.antenna().nrow();

}

PMSCacheVolMeter::~PMSCacheVolMeter() {}


void PMSCacheVolMeter::reset() {
  nDDID_=0;
  nPerDDID_.resize();
  nRowsPerDDID_.resize();
  nChanPerDDID_.resize();
  nCorrPerDDID_.resize();
  nAnt_=0;
}

  
void PMSCacheVolMeter::add(Int DDID,Int nRows) {
  ++nPerDDID_(DDID);
  nRowsPerDDID_(DDID)+=nRows;
}

void PMSCacheVolMeter::add(const VisBuffer& vb) {
  this->add(vb.dataDescriptionId(),vb.nRow());
}

String PMSCacheVolMeter::evalVolume(map<PMS::Axis,Bool> axes, Vector<Bool> axesmask) {

  /*
  cout << "nPerDDID_     = " << nPerDDID_ << endl;
  cout << "nRowsPerDDID_ = " << nRowsPerDDID_ << endl;
  cout << "nChanPerDDID_ = " << nChanPerDDID_ << endl;
  cout << "nCorrPerDDID_ = " << nCorrPerDDID_ << endl;

  cout << "Product = " << nRowsPerDDID_*nChanPerDDID_*nCorrPerDDID_ << endl;
  cout << "Sum     = " << sum(nRowsPerDDID_*nChanPerDDID_*nCorrPerDDID_) << endl;

  cout << "sizeof(Int)    = " << sizeof(Int) << endl;
  cout << "sizeof(Long)   = " << sizeof(Long) << endl;
  cout << "sizeof(uInt64)   = " << sizeof(uInt64) << endl;
  cout << "sizeof(Float)  = " << sizeof(Float) << endl;
  cout << "sizeof(Double) = " << sizeof(Double) << endl;
  */

  uInt64 totalVol(0);
  for (map<PMS::Axis,Bool>::iterator pAi=axes.begin();
       pAi!=axes.end(); ++pAi) {
    if (pAi->second) {
      uInt64 axisVol(0);
      switch(pAi->first) {
      case PMS::SCAN:
      case PMS::FIELD:
      case PMS::SPW:
	axisVol=sizeof(Int)*sum(nPerDDID_);
	break;
      case PMS::TIME:
      case PMS::TIME_INTERVAL:
	axisVol=sizeof(Double)*sum(nPerDDID_);
	break;
      case PMS::CHANNEL:
	axisVol=sizeof(Int)*sum(nPerDDID_*nChanPerDDID_);
	break;
      case PMS::FREQUENCY:
      case PMS::VELOCITY:
	axisVol=sizeof(Double)*sum(nPerDDID_*nChanPerDDID_);
	break;
      case PMS::CORR:
	axisVol=sizeof(Int)*sum(nPerDDID_*nCorrPerDDID_);
	break;
      case PMS::ANTENNA1:
      case PMS::ANTENNA2:
      case PMS::BASELINE:
	axisVol=sizeof(Int)*sum(nRowsPerDDID_);
	break;
      case PMS::UVDIST:
      case PMS::U:
      case PMS::V:
      case PMS::W:
	axisVol=sizeof(Double)*sum(nRowsPerDDID_);
	break;
      case PMS::UVDIST_L:
	axisVol=sizeof(Double)*sum(nRowsPerDDID_*nChanPerDDID_);
	break;
      case PMS::AMP:
      case PMS::PHASE:
      case PMS::REAL:
      case PMS::IMAG:
	axisVol=sizeof(Float)*sum(nRowsPerDDID_*nChanPerDDID_*nCorrPerDDID_);
	break;
      case PMS::FLAG:
	axisVol=sizeof(Bool)*sum(nRowsPerDDID_*nChanPerDDID_*nCorrPerDDID_);
	break;
      case PMS::FLAG_ROW:
	axisVol=sizeof(Bool)*sum(nRowsPerDDID_);
	break;
      case PMS::WT:
	axisVol=sizeof(Int)*sum(nRowsPerDDID_);
	break;
      case PMS::AZ0:
      case PMS::EL0:
      case PMS::HA0:
      case PMS::PA0:
	axisVol=sizeof(Double)*sum(nPerDDID_);
	break;
      case PMS::ANTENNA:
	axisVol=sizeof(Int)*nAnt_*sum(nPerDDID_);
	break;
      case PMS::AZIMUTH:
      case PMS::ELEVATION:
	axisVol=sizeof(Double)*nAnt_*sum(nPerDDID_);
	break;
      case PMS::PARANG:
	axisVol=sizeof(Float)*nAnt_*sum(nPerDDID_);
	break;
      case PMS::ROW:
	axisVol=sizeof(uInt)*sum(nRowsPerDDID_);
	break;
      default: break;
      } // switch
      totalVol+=axisVol;
      //      cout << " " << PMS::axis(pAi->first) << " volume = " << axisVol << " bytes." << endl;
    } 
  } // for 

  
  

  // Add in the plotting mask
  //  (TBD: only if does not reference the flags) 
  if (True) {  // ntrue(axesmask)<2) {
    Vector<uInt64> nplmaskPerDDID(nDDID_,0);
    nplmaskPerDDID(nPerDDID_>uInt64(0))=1;
    if (axesmask(0)) nplmaskPerDDID*=nCorrPerDDID_;
    if (axesmask(1)) nplmaskPerDDID*=nChanPerDDID_;
    if (axesmask(2)) nplmaskPerDDID*=nRowsPerDDID_;
    if (axesmask(3)) nplmaskPerDDID*=uInt64(nAnt_);
    Int plmaskVol=sizeof(Bool)*sum(nplmaskPerDDID);
    //    cout << " Collapsed flag (plot mask) volume = " << plmaskVol << " bytes." << endl;
    totalVol+=plmaskVol;
  }

  // Finally, count the total points for the plot:
  Vector<uInt64> nPointsPerDDID(nDDID_,0);
  nPointsPerDDID(nPerDDID_>uInt64(0))=1;
  if (axesmask(0)) nPointsPerDDID*=nCorrPerDDID_;
  if (axesmask(1)) nPointsPerDDID*=nChanPerDDID_;
  if (axesmask(2)) nPointsPerDDID*=nRowsPerDDID_;
  if (axesmask(3)) nPointsPerDDID*=uInt64(nAnt_);
  Int totalPoints=sum(nPointsPerDDID);

  Double totalVolGB=Double(totalVol)/1.0e9;  // in GB
  Double bytesPerPt=Double(totalVol)/Double(totalPoints);  // bytes/pt

  // Detect if "free" memory should be considered
  String arcpmsif("");
  Bool ignoreFree=(Aipsrc::find(arcpmsif,"plotms.ignorefree") && arcpmsif=="T");

  // Memory info from HostInfo
  uInt hostMemTotalKB=uInt(HostInfo::memoryTotal(true));
  uInt hostMemFreeKB=uInt(HostInfo::memoryFree());

  /*
  cout << "HostInfo::memoryTotal(false) = " << HostInfo::memoryTotal(false) << endl;
  cout << "HostInfo::memoryTotal(true)  = " << hostMemTotalKB << endl;
  cout << "HostInfo::memoryFree()       = " << hostMemFreeKB << endl;
  cout << boolalpha;
  cout << "arcpmsif   = " << arcpmsif << endl;
  cout << "ignoreFree = " << ignoreFree << endl;
  */

  // Memory available to plotms is the min of user's casarc and free
  Double hostMemGB=Double(min(hostMemTotalKB,hostMemFreeKB))/1.0e6; // in GB
  // Override usual calculation if ignoreFree
  if (ignoreFree)
    hostMemGB=Double(hostMemTotalKB)/1.0e6;

  Double fracMem=100.0*totalVolGB/hostMemGB;  // fraction require in %

  stringstream ss;

  if (ignoreFree)
    ss << "Use of 'plotms.ignorefree: T' in the .casarc file may cause" << endl
       << "your machine to swap for very large plots." << endl;
  ss << "Data selection will yield a total of " << totalPoints 
     << " plottable points (flagged and unflagged)." << endl
     << "The plotms cache will require an estimated " 
     << totalVolGB << " GB of memory (" << bytesPerPt << " bytes/point)." << endl
     << "This is " << fracMem << "% of the memory avail. to CASA (" 
     << ((ignoreFree||(hostMemTotalKB<hostMemFreeKB)) ? "total=" : "free=") 
     << hostMemGB << " GB).";

  if (totalVolGB>hostMemGB) {
    ss << endl << "Insufficient memory!";
    throw(AipsError(ss.str()));
  }

  return ss.str();

}


PlotMSCache2::PlotMSCache2(PlotMSApp* parent):
  plotms_(parent),
  indexer_(),
  nChunk_(0),
  refTime_p(0.0),
  nAnt_(0),
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
  netAxesMask_(4,False),
  plmask_()
{

    // Set up loaded axes to be initially empty, and set up data columns for
    // data-based axes.
    const vector<PMS::Axis>& axes = PMS::axes();
    for(unsigned int i = 0; i < axes.size(); i++) {
        loadedAxes_[axes[i]] = false;
        if(PMS::axisIsData(axes[i]))
            loadedAxesData_[axes[i]]= PMS::DEFAULT_DATACOLUMN;
    }    
}

PlotMSCache2::~PlotMSCache2() {

  //  cout << "PMSC2::~PMSC2" << endl;

  // Deflate everything
  deleteIndexer();
  deletePlotMask();
  deleteCache();
}

vector<pair<PMS::Axis, unsigned int> > PlotMSCache2::loadedAxes() const {    
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






void PlotMSCache2::load(const vector<PMS::Axis>& axes,
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

  // Remember the axes:
  currentX_=axes[0];
  currentY_=axes[1];

  // Check if scr cols present
  Bool scrcolOk(False);
  {
    const ColumnDescSet cds=Table(msname).tableDesc().columnDescSet();
    scrcolOk=cds.isDefined("CORRECTED_DATA");

  }

  // Get various names 	 
  { 	 
    MeasurementSet ms(msname_);
    ROMSColumns msCol(ms);
    antnames_.resize();
    stanames_.resize();
    antstanames_.resize();
    fldnames_.resize();
    antnames_=msCol.antenna().name().getColumn(); 	 
    stanames_=msCol.antenna().station().getColumn(); 	 
    fldnames_=msCol.field().name().getColumn(); 	 
    
    antstanames_=antnames_+String("@")+stanames_;

    vm_.reset(); // ensures assign will work!
    vm_= PMSCacheVolMeter(ms,averaging_);
  } 	 

  if (averaging_.anyAveraging()) {
    if (axes[0] == (PMS::WT) |
	axes[1] == (PMS::WT)) {
      throw(AipsError("Sorry, the Wt axes options do not yet support averaging."));
    }
  }

  stringstream ss;
  ss << "Caching for the new plot: " 
     << PMS::axis(axes[1]) << "(" << axes[1] << ") vs. " 
     << PMS::axis(axes[0]) << "(" << axes[0] << ")...\n";
  ss << "Caching for the new plot: " 
     << PMS::axis(currentY_) << "(" << currentY_ << ") vs. " 
     << PMS::axis(currentX_) << "(" << currentX_ << ")...\n";

  logLoad(ss.str());

  if (!scrcolOk) 
    logLoad("NB: Scratch columns not present; will use DATA exclusively.");

  // Calculate which axes need to be loaded; those that have already been
  // loaded do NOT need to be reloaded (assuming that the rest of PlotMS has
  // done its job and cleared the cache if the underlying MS/selection has
  // changed).
  vector<PMS::Axis> loadAxes; vector<PMS::DataColumn> loadData;

  // A map that keeps track of all pending loaded axes
  map<PMS::Axis,Bool> pendingLoadAxes;



  
  // Check meta-data.
  for(unsigned int i = 0; i < N_METADATA; i++) {
    pendingLoadAxes[METADATA[i]]=true; // all meta data will be loaded
    if(!loadedAxes_[METADATA[i]]) {
      loadAxes.push_back(METADATA[i]);
      loadData.push_back(PMS::DEFAULT_DATACOLUMN);
    }
  }


  // Ensure all _already-loaded_ axes are in the pending list
  for (Int i= 0;i<PMS::NONE;++i)
    if (loadedAxes_[PMS::Axis(i)]) pendingLoadAxes[PMS::Axis(i)]=true;
        
  // Check given axes.  Should only be added to load list if: 1) not
  // already in load list, 2) not loaded, or 3) loaded but with different
  // data column (if applicable).


  bool found; PMS::Axis axis; PMS::DataColumn dc;
  for(unsigned int i = 0; i < axes.size(); i++) {
    found = false;
    axis = axes[i];
    
    // add to pending list
    pendingLoadAxes[axis]=true;

    // if data vector is not the same length as axes vector, assume
    // default data column
    dc = PMS::DEFAULT_DATACOLUMN;
    if(i < data.size() && scrcolOk) dc = data[i];

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
  
  if(loadAxes.size() > 0) {
  
    // Now Load data.
    
    // Setup the selected Visiter (getting sort right to support averaging)
    /*
      thread->setStatus("Applying MS selection. Please wait...");
      thread->setAllowedOperations(false,false,false);
      thread->setProgress(1);
    */
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
      
      countChunks(viter,nIterPerAve,averaging,thread);
      trapExcessVolume(pendingLoadAxes);
      loadChunks(viter,averaging,nIterPerAve,
		 loadAxes,loadData,thread);
      
    }
    else {
      
      // supports only channel averaging...    
      countChunks(viter,thread);
      trapExcessVolume(pendingLoadAxes);
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

    if (rvi_p)
      delete rvi_p;
    wvi_p=NULL;
    rvi_p=NULL;
    
  } // something to load
  
  // Setup/revis masks that we use to realize axes relationships
  Vector<Bool> xmask(4,False);
  Vector<Bool> ymask(4,False);
  setAxesMask(currentX_,xmask);
  setAxesMask(currentY_,ymask);
  netAxesMask_=(xmask || ymask);
  
  /*
  cout << boolalpha;
  cout << "xmask = " << xmask << endl;
  cout << "ymask = " << ymask << endl;
  cout << "netAxesMask_ = " << netAxesMask_ << endl;
  */

  // Generate the plot mask from scratch
  setPlotMask();
  
  // At this stage, data is loaded and ready for indexing then plotting....
  dataLoaded_ = true;
  
  logLoad("Finished loading.");
}

void PlotMSCache2::clear() {
  logLoad("Clearing the existing plotms cache.");
  deleteIndexer();
  deletePlotMask();
  deleteCache();
  refTime_p=0.0;
  dataLoaded_=False;
}

#define PMSC_DELETE(VAR)                                                \
  for(unsigned int j = 0; j < VAR.size(); j++)  			\
    if(VAR[j]) delete VAR[j];                                           \
  VAR.resize(0,True);

void PlotMSCache2::release(const vector<PMS::Axis>& axes) {
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

        case PMS::WT: PMSC_DELETE(wt_) break;

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
        
        if(dataLoaded_ &&
           (currentX_ == axes[i] || currentY_ == axes[i])) {
            dataLoaded_ = false;
        }
    }
    
    if(!dataLoaded_) nChunk_ = 0;
}

void PlotMSCache2::setUpIndexer(PMS::Axis iteraxis, Bool globalXRange, Bool globalYRange) {

  logLoad("Setting up iteration indexing (if necessary), and calculating plot ranges.");

  //  cout << "############ PlotMSCache2::setUpIndexer: " << PMS::axis(iteraxis) 
  //       << " cacheReady() = " << boolalpha << cacheReady() << endl;

  Int nIter=0;
  Vector<Int> iterValues;

  // If the cache hasn't been filled, do nothing
  if (!cacheReady()) return;

  switch (iteraxis) {
  case PMS::SCAN: {
    iterValues=scan_(goodChunk_).getCompressedArray();
    nIter=genSort(iterValues,(Sort::QuickSort | Sort::NoDuplicates));
    break;
  }
  case PMS::SPW: {
    iterValues=spw_(goodChunk_).getCompressedArray();
    nIter=genSort(iterValues,(Sort::QuickSort | Sort::NoDuplicates));
    break;
  }
  case PMS::FIELD: {
    iterValues=field_(goodChunk_).getCompressedArray();
    nIter=genSort(iterValues,(Sort::QuickSort | Sort::NoDuplicates));
    break;
  }
  case PMS::BASELINE: {

    // Revise axes mask, etc., to ensure baseline-dependence
    if (!netAxesMask_(2)) {
      netAxesMask_(2)=True;
      setPlotMask();
    }

    // Maximum possible baselines (includes ACs)
    Int nBslnMax((nAnt_+1)*(nAnt_+2)/2);

    // Find the limited list of _occurring_ baseline indices
    Vector<Int> bslnList(nBslnMax);
    Vector<Bool> bslnMask(nBslnMax,False);
    indgen(bslnList);
    /*
    cout << "*baseline_[0] = " << *baseline_[0] << endl;
    cout << "*antenna1_[0] = " << *antenna1_[0] << endl;
    cout << "*antenna2_[0] = " << *antenna2_[0] << endl;
    cout << "bslnList = " << bslnList << endl;
    */

    for (Int ich=0;ich<nChunk_;++ich)
      if (goodChunk_(ich))
	for (Int ibl=0;ibl<chunkShapes()(2,ich);++ibl)
	  bslnMask(*(baseline_[ich]->data()+ibl))=True;
    //    cout << "bslnMask = " << boolalpha << bslnMask << endl;

    // Remember only the occuring baseline indices
    iterValues=bslnList(bslnMask).getCompressedArray();
    nIter=iterValues.nelements();
    //    cout << "nIter = " << nIter << " iterValues = " << iterValues << endl;

    break;
  }
  case PMS::ANTENNA: {

    // Escape if (full) baseline averaging is on, since we won't find any iterations
    if (averaging_.baseline())
      throw(AipsError("Iteration over baseline not supported with full baseline averaging."));

    // Revise axes mask, etc., to ensure baseline-dependence
    if (!netAxesMask_(2)) {
      netAxesMask_(2)=True;
      setPlotMask();
    }

    // Find the limited list of _occuring_ antenna indices
    Vector<Int> antList(nAnt_);
    Vector<Bool> antMask(nAnt_,False);
    indgen(antList);
    for (Int ich=0;ich<nChunk_;++ich)
      if (goodChunk_(ich))
	for (Int ibl=0;ibl<chunkShapes()(2,ich);++ibl) {
	  Int a1=*(antenna1_[ich]->data()+ibl);
	  Int a2=*(antenna2_[ich]->data()+ibl);
	  if (a1>-1) antMask(a1)=True;
	  if (a2>-1) antMask(a2)=True;
	}
    // Remember only the occuring antenna indices
    iterValues=antList(antMask).getCompressedArray();
    nIter=iterValues.nelements();
    break;
  }
  case PMS::NONE: {
    nIter=1;
    iterValues.resize(1);
    iterValues(0)=0;
    break;
  }
  default:
    throw(AipsError("Unsupported iteration axis:"+PMS::axis(iteraxis)));
    break;
  }      

  if (iteraxis!=PMS::NONE) {
    stringstream ss;
    ss << "Found " << nIter << " " << PMS::axis(iteraxis) << " iterations.";
    logLoad(ss.str());
  }
  
  //  cout << "********nIter = " << nIter << " iterValues = " << iterValues(IPosition(1,0),IPosition(1,nIter-1)) << endl;

  deleteIndexer();
  indexer_.resize(nIter);
  indexer_.set(NULL);
  for (Int iter=0;iter<nIter;++iter)
    indexer_[iter] = new PlotMSIndexer(this,currentX_,currentY_,iteraxis,iterValues(iter));    

  // Extract global ranges from the indexers
  // Initialize limits
  xminG_=yminG_=xflminG_=yflminG_=DBL_MAX;
  xmaxG_=ymaxG_=xflmaxG_=yflmaxG_=-DBL_MAX;
  Double ixmin,iymin,ixmax,iymax;
  for (Int iter=0;iter<nIter;++iter) {
    indexer_[iter]->unmaskedMinsMaxesRaw(ixmin,ixmax,iymin,iymax);
    xminG_=min(xminG_,ixmin);
    xmaxG_=max(xmaxG_,ixmax);
    yminG_=min(yminG_,iymin);
    ymaxG_=max(ymaxG_,iymax);
    indexer_[iter]->maskedMinsMaxesRaw(ixmin,ixmax,iymin,iymax);
    xflminG_=min(xflminG_,ixmin);
    xflmaxG_=max(xflmaxG_,ixmax);
    yflminG_=min(yflminG_,iymin);
    yflmaxG_=max(yflmaxG_,iymax);
    // set usage of globals
    indexer_[iter]->setGlobalMinMax(globalXRange,globalYRange);
  }

  {
    stringstream ss;
    ss << "Global ranges:" << endl 
       << PMS::axis(currentX_) << ": " 
       << xminG_ << "-" << xmaxG_ << " (unflagged); "
       << xflminG_ << "-" << xflmaxG_ << " (flagged)." << endl
       << PMS::axis(currentY_) << ": " 
       << yminG_ << "-" << ymaxG_ << " (unflagged); "
       << yflminG_ << "-" << yflmaxG_ << "(flagged).";
    logLoad(ss.str());
    //  cout << "Use global ranges? : " << boolalpha << globalXRange << " " << globalYRange << endl;
  }
}


//*********************************
// protected method implementations



// increase the number of chunks we can store
void PlotMSCache2::increaseChunks(Int nc) {

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

  wt_.resize(nChunk_,False,True);

  az0_.resize(nChunk_,True);
  el0_.resize(nChunk_,True);
  ha0_.resize(nChunk_,True);
  pa0_.resize(nChunk_,True);
  
  antenna_.resize(nChunk_,False,True);
  az_.resize(nChunk_,False,True);
  el_.resize(nChunk_,False,True);
  parang_.resize(nChunk_,False,True);

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
    wt_[ic] = new Matrix<Float>();
    antenna_[ic] = new Vector<Int>();
    az_[ic] = new Vector<Double>();
    el_[ic] = new Vector<Double>();
    parang_[ic] = new Vector<Float>();
  }
}

void PlotMSCache2::deleteCache() {
    // Release all axes.
    release(PMS::axes());

    // zero the meta-name containers
    antnames_.resize();
    stanames_.resize();
    antstanames_.resize();
    fldnames_.resize();

}
void PlotMSCache2::deleteIndexer() {
  for (uInt i=0;i<indexer_.nelements();++i) 
    if (indexer_[i]) delete indexer_[i];
  indexer_.resize(0,True);
}



void PlotMSCache2::setUpVisIter(const String& msname,
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


      
void PlotMSCache2::countChunks(ROVisibilityIterator& vi,
			       PlotMSCacheThread* ) {  // thread) {

  /*
  if (thread!=NULL) {
    thread->setStatus("Establishing cache size.  Please wait...");
    thread->setAllowedOperations(false,false,false);
  }
  */

  // This is the old way, with no averaging over chunks.

  VisBuffer vb(vi);
  
  vi.originChunks();
  vi.origin();
  refTime_p=86400.0*floor(vb.time()(0)/86400.0);

  // Count number of chunks.
  int chunk = 0;
  for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

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

    for (vi.origin(); vi.more(); vi++) {
      ++chunk;
      vm_.add(vb);
    }
  }
  if(chunk != nChunk_) increaseChunks(chunk);
  
  //  cout << "Found " << nChunk_ << " " << chunk << " chunks." << endl;

}


void PlotMSCache2::countChunks(ROVisibilityIterator& vi, Vector<Int>& nIterPerAve,
			       const PlotMSAveraging& averaging,
			       PlotMSCacheThread* ) { //  thread) {

  /*
  if (thread!=NULL) {
    thread->setStatus("Establishing cache size.  Please wait...");
    thread->setAllowedOperations(false,false,false);
  }
  */
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
  Int thisddid(-1),lastddid(-1);
  Int maxAveNRows(0);
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
    Int iter(0);
    for (vi.origin(); vi.more();vi++,iter++) {

      time1=vb.time()(0);  // first time in this vb
      thisscan=vb.scan()(0);
      thisfld=vb.fieldId();
      thisspw=vb.spectralWindow();
      thisddid=vb.dataDescriptionId();

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

	// If we have accumulated enough info, poke the volume meter,
	//  with the _previous_ info, and reset the ave'd row counter
	if (ave>-1) {
	  vm_.add(lastddid,maxAveNRows);
	  maxAveNRows=0;
	}


	avetime1=time1;  // for next go
	ave++;
	
	if (verby) ss << "ave = " << ave << "\n";


	// increase size of nIterPerAve array, if needed
	if (nIterPerAve.nelements()<uInt(ave+1))
	  nIterPerAve.resize(nIterPerAve.nelements()+100,True);
	nIterPerAve(ave)=0;
      }
      
      // Keep track of the maximum # of rows that might get averaged
      maxAveNRows=max(maxAveNRows,vb.nRow());


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
      lastddid=thisddid;
    }
  }
  // Add in the last iteration
  vm_.add(lastddid,maxAveNRows);
  
  Int nAve(ave+1);
  nIterPerAve.resize(nAve,True);
  
  if (verby)  ss << "nIterPerAve = " << nIterPerAve;
  if (verby) logInfo("count_chunks", ss.str());


  if (nChunk_ != nAve) increaseChunks(nAve);
  
}

void PlotMSCache2::trapExcessVolume(map<PMS::Axis,Bool> pendingLoadAxes) {
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



 
void PlotMSCache2::loadChunks(ROVisibilityIterator& vi,
			     const vector<PMS::Axis> loadAxes,
			     const vector<PMS::DataColumn> loadData,
			     const PlotMSAveraging& averaging,
			     PlotMSCacheThread* thread) {

  // permit cancel in progress meter:
  if(thread != NULL)
    thread->setAllowedOperations(false,false,true);
    
  logLoad("Loading chunks......");
  VisBuffer vb(vi);

  // Initialize the freq/vel calculator (in case we use it)
  vbu_=VisBufferUtil(vb);

  Int chunk = 0;
  chshapes_.resize(4,nChunk_);
  goodChunk_.resize(nChunk_);
  goodChunk_.set(False);
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
      goodChunk_(chunk)=True;

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

void PlotMSCache2::loadChunks(ROVisibilityIterator& vi,
			     const PlotMSAveraging& averaging,
			     const Vector<Int>& nIterPerAve,
			     const vector<PMS::Axis> loadAxes,
			     const vector<PMS::DataColumn> loadData,
			     PlotMSCacheThread* thread) {
  
  // permit cancel in progress meter:
  if(thread != NULL)
    thread->setAllowedOperations(false,false,true);

  logLoad("Loading chunks with averaging.....");

  Bool verby(False);

  VisBuffer vb(vi);

  // Initialize the freq/vel calculator (in case we use it)
  vbu_=VisBufferUtil(vb);

  chshapes_.resize(4,nChunk_);
  goodChunk_.resize(nChunk_);
  goodChunk_.set(False);
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

    // Only if the average yielded some data
    if (avb.nRow()>0) {

      // Form Stokes parameters, if requested
      if (transformations_.formStokes())
	avb.formStokes();
      
      // Cache the data shapes
      chshapes_(0,chunk)=avb.nCorr();
      chshapes_(1,chunk)=avb.nChannel();
      chshapes_(2,chunk)=avb.nRow();
      chshapes_(3,chunk)=vi.numberAnt();
      goodChunk_(chunk)=True;

      for(unsigned int i = 0; i < loadAxes.size(); i++) {
	loadAxis(avb, chunk, loadAxes[i], loadData[i]);
      }
    }
    else {
      // no points in this chunk
      goodChunk_(chunk)=False;
      chshapes_.column(chunk)=0;
    }

    // If a thread is given, update it.
    if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
			  chunk % THREAD_SEGMENT == 0)) {
      progress = ((double)chunk+1) / nChunk_;
      thread->setProgress((unsigned int)((progress * 100) + 0.5));
    }
  }

  //  cout << boolalpha << "goodChunk_ = " << goodChunk_ << endl;


}

void PlotMSCache2::forceVBread(VisBuffer& vb,
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

void PlotMSCache2::discernData(vector<PMS::Axis> loadAxes,
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


void PlotMSCache2::setAxesMask(PMS::Axis axis,Vector<Bool>& axismask) {

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
  default:
    break;
  }

}

Vector<Bool> PlotMSCache2::netAxesMask(PMS::Axis xaxis,PMS::Axis yaxis) {

  if (xaxis==PMS::NONE || yaxis==PMS::NONE)
    throw(AipsError("Problem in PlotMSCache2::netAxesMask()."));

  Vector<Bool> xmask(4,False);
  setAxesMask(xaxis,xmask);
  Vector<Bool> ymask(4,False);
  setAxesMask(yaxis,ymask);

  return (xmask || ymask);

}


void PlotMSCache2::setPlotMask() {

  logLoad("Generating the plot mask.");

  // Generate the plot mask
  deletePlotMask();
  plmask_.resize(nChunk());
  plmask_.set(NULL);
  for (Int ichk=0;ichk<nChunk();++ichk) {
    plmask_[ichk] = new Array<Bool>();
    // create a collapsed version of the flags for this chunk
    setPlotMask(ichk);    
  }

}


void PlotMSCache2::setPlotMask(Int chunk) {

  // Do nothing if chunk empty
  if (!goodChunk_(chunk))
    return;

  IPosition nsh(3,1,1,1),csh;
  
  for (Int iax=0;iax<3;++iax) {
    if (netAxesMask_(iax)) 
      // non-trivial size for this axis
      nsh(iax)=chunkShapes()(iax,chunk);
    else 
      // add this axis to collapse list
      csh.append(IPosition(1,iax));
  }
  
  if (netAxesMask_(3) && !netAxesMask_(2)) {
    nsh(2)=chunkShapes()(3,chunk);   // antenna axis length
    plmask_[chunk]->resize(nsh);
    // TBD: derive antenna flags from baseline flags
    plmask_[chunk]->set(True);   
  }
  else {
    plmask_[chunk]->resize(nsh);
    (*plmask_[chunk]) = operator>(partialNFalse(*flag_[chunk],csh).reform(nsh),uInt(0));
  }

}

void PlotMSCache2::deletePlotMask() {

  for (uInt i=0;i<plmask_.nelements();++i)
    if (plmask_[i]) delete plmask_[i];

  plmask_.resize(0,True);

  // This indexer is no longer ready for plotting
  dataLoaded_=False;

}

void PlotMSCache2::loadAxis(VisBuffer& vb, Int vbnum, PMS::Axis axis,
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
      for (Int irow=0;irow<vb.nRow();++irow) {
	if (a1(irow)<0) a1(irow)=chshapes_(3,0);
	if (a2(irow)<0) a2(irow)=chshapes_(3,0);
	bl(irow)=(chshapes_(3,0)+1)*a1(irow) - (a1(irow) * (a1(irow) + 1)) / 2 + a2(irow);
      }
      break;
    }
    case PMS::UVDIST: {
      Array<Double> u(vb.uvwMat().row(0));
      Array<Double> v(vb.uvwMat().row(1));
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
      Array<Double> u(vb.uvwMat().row(0));
      Array<Double> v(vb.uvwMat().row(1));
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

    case PMS::WT: {
      *wt_[vbnum] = vb.weightMat();
      break;
    }
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
    case PMS::PA0: {
      pa0_(vbnum) = vb.parang0(vb.time()(0))*180.0/C::pi; // in degrees
      if (pa0_(vbnum)<0.0) pa0_(vbnum)+=360.0;
      break;
    }
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

    default:
      throw(AipsError("Axis choice NYI"));
      break;
    }
}

void PlotMSCache2::flagToDisk(const PlotMSFlagging& flagging,
			      Vector<Int>& flchunks, Vector<Int>& flrelids, 
			      Bool flag, PlotMSIndexer* indexer) {

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
	  Int currChunk=flchunks(order[iflag]);
	  Double time=getTime(currChunk,0);
	  Int spw=Int(getSpw(currChunk,0));
	  Int field=Int(getField(currChunk,0));
	  ss << "Time diff: " << time-vb.time()(0) << " "  << time << " " << vb.time()(0) << "\n";
	  ss << "Spw diff:  " << spw-vb.spectralWindow() << " " << spw << " " << vb.spectralWindow() << "\n"; 
	  ss << "Field diff:  " << field-vb.fieldId() << " " << field << " " << vb.fieldId() << "\n";
	}

	// Apply all flags in this chunk to this VB
	ifl=iflag;
	while (ifl<Int(nflag) && flchunks(order[ifl])==ichk) {

	  Int currChunk=flchunks(order[ifl]);
	  Int irel=flrelids(order[ifl]);

	  Slice corr,chan,bsln;

	  // Set flag range on correlation axis:
	  if (netAxesMask_(0) && !flagging.corrAll()) {
	    // A specific single correlation
	    Int icorr=indexer->getIndex1000(currChunk,irel);
	    corr=Slice(icorr,1,1);
	  }
	  else
	    corr=Slice(0,ncorr,1);

	  // Set Flag range on channel axis:
	  if (netAxesMask_(1) && !flagging.channel()) {
	    Int ichan=indexer->getIndex0100(currChunk,irel);
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
	  Int thisA1=Int(getAnt1(currChunk,indexer->getIndex0010(currChunk,irel)));
	  Int thisA2=Int(getAnt2(currChunk,indexer->getIndex0010(currChunk,irel)));
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


unsigned int PlotMSCache2::nPointsForAxis(PMS::Axis axis) const {
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
    case PMS::WT:
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
            else if(axis == PMS::WT)       n += wt_[i]->size();
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

void PlotMSCache2::log(const String& method, const String& message,
        int eventType) {
    plotms_->getLogger()->postMessage(PMS::LOG_ORIGIN,method,message,eventType);}

}

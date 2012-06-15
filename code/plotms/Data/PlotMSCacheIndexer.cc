//# PlotMSCacheIndexer.cc: Data cache indexer for plotms.
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
#include <plotms/Data/PlotMSCacheIndexer.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Sort.h>
#include <casa/OS/Timer.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>

namespace casa {

PlotMSCacheIndexer::PlotMSCacheIndexer(PlotMSCache* parent):
  plotmscache_(parent),
  currChunk_(0),
  irel_(0),
  lasti_(-1),
  nPoints_(),
  currentX_(PMS::TIME),
  currentY_(PMS::AMP),
  indexerReady_(False),
  netAxesMask_(4,False),
  icorrmax_(),
  ichanmax_(),
  ibslnmax_(),
  idatamax_(),
  nperchan_(),
  nperbsln_(),
  nperant_(),
  ichanbslnmax_(),
  iantmax_(),
  plmask_(),
  xmin_(DBL_MAX),
  ymin_(DBL_MAX),
  xflmin_(DBL_MAX),
  yflmin_(DBL_MAX),
  xmax_(-DBL_MAX),
  ymax_(-DBL_MAX),
  xflmax_(-DBL_MAX),
  yflmax_(-DBL_MAX)
{
  // Nothing for now
}

PlotMSCacheIndexer::~PlotMSCacheIndexer() {

  // Delete the plot mask
  deleteMask();

}


void PlotMSCacheIndexer::setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis) {

  // Remember the chosen axes
  currentX_ = xAxis; currentY_ = yAxis;

  // Set up masks that we use to realize axes relationships
  Vector<Bool> xmask(4,False);
  Vector<Bool> ymask(4,False);
  getAxesMask(xAxis,xmask);
  getAxesMask(yAxis,ymask);

  netAxesMask_=(xmask || ymask);

  // cout << boolalpha;
  //  cout << "xmask = " << xmask << endl;
  //  cout << "ymask = " << ymask << endl;
  // cout << "netAxesMask_ = " << netAxesMask_ << endl;


  // Forbid antenna-based/baseline-based combination plots, for now
  //  (e.g., data vs. _antenna-based_ elevation)
  if (netAxesMask_(2)&&netAxesMask_(3))
    throw(AipsError("Cannot yet support antenna-based and baseline-based data in same plot."));
 
  Matrix<Int>& chsh(this->chunkShapes());

  cout << "Set shapes..." << flush;

  icorrmax_.reference(chsh.row(0));
  ichanmax_.reference(chsh.row(1));
  ibslnmax_.reference(chsh.row(2));
  iantmax_.reference(chsh.row(3));
  
  idatamax_.resize(nChunk());
  idatamax_=chsh.row(0);
  idatamax_*=chsh.row(1);
  idatamax_*=chsh.row(2);
  
  ichanbslnmax_.resize(nChunk());
  ichanbslnmax_=chsh.row(1);
  ichanbslnmax_*=chsh.row(2);
  
  nperchan_.resize(nChunk());
  nperchan_.set(1);
  if (netAxesMask_(0)) nperchan_*=chsh.row(0);
  
  nperbsln_.resize(nChunk());
  nperbsln_.set(1);
  if (netAxesMask_(0)) nperbsln_*=chsh.row(0);
  if (netAxesMask_(1)) nperbsln_*=chsh.row(1);

  nperant_.reference(nperbsln_);

  cout << "done." << endl << "Set methods..." << flush;


  // Set up method pointers for the chosen axes
  setMethod(getXFromCache_,xAxis);
  setMethod(getYFromCache_,yAxis);

  // And the indexers
  setIndexer(XIndexer_,xAxis);
  setIndexer(YIndexer_,yAxis);

  // And the mask collapsers
  setCollapser(collapseXMask_,xAxis);
  setCollapser(collapseYMask_,yAxis);


  cout << "done." << endl << "Set mask..." << flush;

  plmask_.resize(nChunk());
  plmask_.set(NULL);
  for (Int ichk=0;ichk<nChunk();++ichk) {
    plmask_[ichk] = new Array<Bool>();
    // create a collapsed version of the flags for this chunk
    setPlotMask(ichk);    
  }


  cout << "done." << endl << "Count points..." << flush;

  // Count up the total number of points we will plot
  //   (keep a cumualtive running total)

  nPoints_.resize(nChunk());
  Int cumulativeN(0);
  for (Int ic=0;ic<nChunk();++ic) {
    Int chN=1;
    for (Int ii=0;ii<4;++ii) 
      if (netAxesMask_(ii))
	chN*=chsh.column(ic)(ii);
    cumulativeN+=chN;
    nPoints_(ic)=cumulativeN;
  }

  cout << "done." << endl;

  cout << "nPoints() = " << nPoints() << endl;

  // Compute the nominal plot ranges
  cout << "Computing ranges..." << flush;
  computeRanges();

  // The indexer is now ready for plotting
  indexerReady_ = true;

  cout << "done." << endl;

}

void PlotMSCacheIndexer::getAxesMask(PMS::Axis axis,Vector<Bool>& axismask) {

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

void PlotMSCacheIndexer::setPlotMask(Int chunk) {

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
    nsh(2)=iantmax_(chunk);
    plmask_[chunk]->resize(nsh);
    // TBD: derive antenna flags from baseline flags
    plmask_[chunk]->set(True);   
  }
  else {
    plmask_[chunk]->resize(nsh);
    (*plmask_[chunk]) = operator>(partialNFalse(plotmscache_->flag(chunk),csh).reform(nsh),uInt(0));
  }
  
}

void PlotMSCacheIndexer::deleteMask() {

  for (uInt i=0;i<plmask_.nelements();++i)
    if (plmask_[i]) delete plmask_[i];

  plmask_.resize(0,True);

  // This indexer is no longer ready for plotting
  indexerReady_=False;

}

Double PlotMSCacheIndexer::getX(Int i) {

  // Find correct chunk and index offset
  //  sets currChunk_ and irel_ which are used below
  setChunk(i);

  return getX(currChunk_,
	      (this->*XIndexer_)(currChunk_,irel_));
}

Double PlotMSCacheIndexer::getY(Int i) {

  // Find correct chunk and index offset
  //  sets currChunk_ and irel_ which are used below
  setChunk(i);

  return getY(currChunk_,
	      (this->*YIndexer_)(currChunk_,irel_));  
}

void PlotMSCacheIndexer::getXY(Int i,Double& x,Double& y) {

  // Find correct chunk and index offset
  setChunk(i);

  x= (plotmscache_->*getXFromCache_)(currChunk_,
				     (this->*XIndexer_)(currChunk_,irel_));
  y= (plotmscache_->*getYFromCache_)(currChunk_,
				     (this->*YIndexer_)(currChunk_,irel_));

}



Bool PlotMSCacheIndexer::getFlagMask(Int i) {

  // Return False if point i is UNnmasked

  // Find correct chunk and index offset
  setChunk(i);
  
  // NB: Note that plmask_ is true for unflagged data,
  //     so "not" it
  return !(*(plmask_[currChunk_]->data()+irel_));
}

void PlotMSCacheIndexer::setChunk(Int i) {

  // NB: this method assumes that i>=lasti, for now

  if (i==lasti_)
    // already found this one on previous call (e.g., for mask 
    //   or the other axis), so change nothing
    return;

  // reset to the first chunk if very first point requested
  if (i==0) currChunk_=0;  
  
  // Bump at chunk  boundaries
  //  (this should work for sliced caches where some chunks are empty)
  while (i > (nPoints_(currChunk_)-1)) ++currChunk_;
  
  // TBD:  Back up to a previous chunk?
  //  while (i < (nPoints_(currChunk_)) && currChunk_>0) --currChunk_;
  
  // Calculate the offset into the current chunk
  irel_=i;
  if (currChunk_>0) 
    irel_-=nPoints_(currChunk_-1);

  // Remember this i next time around
  lasti_=i;

}

void PlotMSCacheIndexer::setMethod(PlotMSCacheMemPtr& getmethod,PMS::Axis axis) {

  // Set axis-specific get methods
  switch(axis) {
    // Degenerate ones
  case PMS::SCAN:
    getmethod = &PlotMSCache::getScan;
    break;
  case PMS::FIELD:
    getmethod = &PlotMSCache::getField;
    break;
  case PMS::TIME:
    getmethod = &PlotMSCache::getTime;
    break;
  case PMS::TIME_INTERVAL:
    getmethod = &PlotMSCache::getTimeIntr;
    break;
  case PMS::SPW:
    getmethod = &PlotMSCache::getSpw;
    break;

    // Partial shapes
  case PMS::FREQUENCY:
    getmethod = &PlotMSCache::getFreq;
    break;
  case PMS::CHANNEL:
    getmethod = &PlotMSCache::getChan;
    break;
  case PMS::CORR:
    getmethod = &PlotMSCache::getCorr;
    break;
  case PMS::ANTENNA1:
    getmethod = &PlotMSCache::getAnt1;
    break;
  case PMS::ANTENNA2:
    getmethod = &PlotMSCache::getAnt2;
    break;
  case PMS::BASELINE:
    getmethod = &PlotMSCache::getBsln;
    break;

  case PMS::UVDIST:
    getmethod = &PlotMSCache::getUVDist;
    break;
  case PMS::U:
    getmethod = &PlotMSCache::getU;
    break;
  case PMS::V:
    getmethod = &PlotMSCache::getV;
    break;
  case PMS::W:
    getmethod = &PlotMSCache::getW;
    break;
  case PMS::UVDIST_L:
    getmethod = &PlotMSCache::getUVDistL;
    break;

    // Data
  case PMS::AMP:
    getmethod = &PlotMSCache::getAmp;
    break;
  case PMS::PHASE:
    getmethod = &PlotMSCache::getPha;
    break;
  case PMS::REAL:
    getmethod = &PlotMSCache::getReal;
    break;
  case PMS::IMAG:
    getmethod = &PlotMSCache::getImag;
    break;
  case PMS::FLAG:
    getmethod = &PlotMSCache::getFlag;
    break;

  case PMS::FLAG_ROW:
    getmethod = &PlotMSCache::getFlagRow;
    break;

  case PMS::ANTENNA:
    getmethod = &PlotMSCache::getAntenna;
    break;
  case PMS::AZIMUTH:
    getmethod = &PlotMSCache::getAz;
    break;
  case PMS::ELEVATION:
    getmethod = &PlotMSCache::getEl;
    break;
  case PMS::PARANG:
    getmethod = &PlotMSCache::getParAng;
    break;
  case PMS::ROW:
    getmethod = &PlotMSCache::getRow;
    break;
  default:
    throw(AipsError("Help!"));
  }

}

void PlotMSCacheIndexer::setIndexer(IndexerMethodPtr& indexmethod,PMS::Axis axis) {

  // Set axis-specific indexing method
  switch(axis) {

    // Degenerate axes (no corr-,chan-,bsln,antenna-dependence)
  case PMS::SCAN:
  case PMS::FIELD:
  case PMS::TIME:
  case PMS::TIME_INTERVAL:
  case PMS::SPW:
    indexmethod = &PlotMSCacheIndexer::getIndex0000;
    break;

    // corr-dep
  case PMS::CORR:
    indexmethod = &PlotMSCacheIndexer::getIndex1000;
    break;

    // corr-,chan-dep
    //  case PMS::WEIGHT
    //    indexmethod = &PlotMSCacheIndex::getIndex1100;
    //    break;

    // corr-,bsln-dep
    //  case PMS::WEIGHT
    //    indexmethod = &PlotMSCacheIndex::getIndex1010;
    //    break;

    // corr-,chan-,bsln-dep
  case PMS::AMP:
  case PMS::PHASE:
  case PMS::REAL:
  case PMS::IMAG:
  case PMS::FLAG:
    indexmethod = &PlotMSCacheIndexer::getIndex1110;
    break;

    // chan-dep
  case PMS::FREQUENCY:
  case PMS::CHANNEL:
    indexmethod = &PlotMSCacheIndexer::getIndex0100;
    break;

    // chan-,bsln-dep
  case PMS::UVDIST_L:
    indexmethod = &PlotMSCacheIndexer::getIndex0110;
    break;

    // bsln-dep
  case PMS::ROW:
  case PMS::ANTENNA1:
  case PMS::ANTENNA2:
  case PMS::BASELINE:
  case PMS::UVDIST:
  case PMS::U:
  case PMS::V:
  case PMS::W:
  case PMS::FLAG_ROW:
    indexmethod = &PlotMSCacheIndexer::getIndex0010;
    break;

    // antenna-dep
  case PMS::ANTENNA:
  case PMS::AZIMUTH:
  case PMS::ELEVATION:
  case PMS::PARANG:
    indexmethod = &PlotMSCacheIndexer::getIndex0001;
    break;

  default:
    throw(AipsError("Help! No index method available!"));
  }

}

void PlotMSCacheIndexer::setCollapser(CollapseMethodPtr& collmethod,PMS::Axis axis) {

  // Set axis-specific mask collapsing method
  switch(axis) {

    // Degenerate axes (no corr-,chan-,bsln,antenna-dependence)
  case PMS::SCAN:
  case PMS::FIELD:
  case PMS::TIME:
  case PMS::TIME_INTERVAL:
  case PMS::SPW: {
    collmethod = &PlotMSCacheIndexer::collapseMask0000;
    break;
  }
    // corr-dep
  case PMS::CORR:
    collmethod = &PlotMSCacheIndexer::collapseMask1000;
    break;

    // corr-,chan-dep
    //  case PMS::WEIGHT
    //    collmethod = &PlotMSCacheIndex::collapseMask1100;
    //    break;

    // corr-,bsln-dep
    //  case PMS::WEIGHT
    //    collmethod = &PlotMSCacheIndex::collapseMask1010;
    //    break;

    // corr-,chan-,bsln-dep
  case PMS::AMP:
  case PMS::PHASE:
  case PMS::REAL:
  case PMS::IMAG:
  case PMS::FLAG:
    collmethod = &PlotMSCacheIndexer::collapseMask1110;
    break;

    // chan-dep
  case PMS::FREQUENCY:
  case PMS::CHANNEL:
    collmethod = &PlotMSCacheIndexer::collapseMask0100;
    break;

    // chan-,bsln-dep
  case PMS::UVDIST_L:
    collmethod = &PlotMSCacheIndexer::collapseMask0110;
    break;

    // bsln-dep
  case PMS::ROW:
  case PMS::ANTENNA1:
  case PMS::ANTENNA2:
  case PMS::BASELINE:
  case PMS::UVDIST:
  case PMS::U:
  case PMS::V:
  case PMS::W:
  case PMS::FLAG_ROW:
    collmethod = &PlotMSCacheIndexer::collapseMask0010;
    break;

    // antenna-dep
  case PMS::ANTENNA:
  case PMS::AZIMUTH:
  case PMS::ELEVATION:
  case PMS::PARANG:
    collmethod = &PlotMSCacheIndexer::collapseMask0001;
    break;

  default:
    throw(AipsError("Help! No index method available!"));
  }

}


void PlotMSCacheIndexer::getRanges(Double& minX, Double& maxX, Double& minY,
        Double& maxY) {

  // If none available here, do nothing
  if(!indexerReady_) return;

  minX = xmin_; maxX = xmax_;
  minY = ymin_; maxY = ymax_;
}


PlotLogMessage* PlotMSCacheIndexer::locateRange(const Vector<PlotRegion>& regions) {

  Timer locatetimer;
  locatetimer.mark();
  
  Double thisx, thisy;
  stringstream ss;
  Int nFound = 0, n = nPoints();
  
  for(Int i = 0; i < n; i++) {

    if (!getFlagMask(i)) {

      getXY(i, thisx, thisy);
      
      for(uInt j = 0; j < regions.size(); j++) {
	if (thisx > regions[j].left() && thisx < regions[j].right() &&
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


void PlotMSCacheIndexer::reportMeta(Double x, Double y,stringstream& ss) {

  // This method assumes currChunk_ and irel_ already set correctly!

  ss << "Scan=" << plotmscache_->getScan(currChunk_,0) << " ";
  ss << "Field=" << plotmscache_->getField(currChunk_,0) << " ";
  ss << "Time=" << MVTime(plotmscache_->getTime(currChunk_,0)/C::day).string(MVTime::YMD,7) << " ";
  ss << "BL=";

  Int ant1=Int( plotmscache_->getAnt1(currChunk_,getIndex0010(currChunk_,irel_)) );
  if (!netAxesMask_(2) || ant1<0)
    ss << "*-";
  else
    ss << ant1 << "-";

  Int ant2=Int( plotmscache_->getAnt2(currChunk_,getIndex0010(currChunk_,irel_)) );
  if (!netAxesMask_(2) || ant2<0)
    ss << "* ";
  else
    ss << ant2 << " ";

  Int spw=Int(plotmscache_->getSpw(currChunk_,0));
  ss << "Spw=";
  if (spw<0)
    ss << "* ";
  else
    ss << spw << " ";

  Int ichan=getIndex0100(currChunk_,irel_);

  ss << "Chan=";
  if (netAxesMask_(1)) {

    PlotMSAveraging& pmsave(plotmscache_->averaging());
    if (pmsave.channel() && pmsave.channelValue()>1) {
      Int& lochan=plotmscache_->chanAveBounds(spw)(ichan,0);
      Int& hichan=plotmscache_->chanAveBounds(spw)(ichan,1);
      ss << "<" << lochan << "~" << hichan  << "> ";
    }
    else
      ss << Int(plotmscache_->getChan(currChunk_,ichan)) << " ";
  }
  else
    ss << "*  ";

  ss << "Freq=";
  if (netAxesMask_(1))
    ss << plotmscache_->getFreq(currChunk_,ichan) << " ";
  else
    ss << "*        ";

  ss << "Corr=";
  if (netAxesMask_(0))
    ss << plotmscache_->getCorr(currChunk_,getIndex1000(currChunk_,irel_)) << " ";
  else
    ss << "* ";
  ss << "X=" << x << " ";
  ss << "Y="  << y << " ";
  ss << "(" << (currChunk_ > 0 ? (nPoints_(currChunk_-1)+irel_) : irel_) << "/";
  ss << currChunk_ << "/" << irel_ << ")";

}

PlotLogMessage* PlotMSCacheIndexer::flagRange(const PlotMSFlagging& flagging,
        const Vector<PlotRegion>& regions, Bool flag) {
  Timer flagtimer;
  flagtimer.mark();
  
  return new PlotLogMessage(PMS::LOG_ORIGIN,
			    flag ? PMS::LOG_ORIGIN_FLAG : PMS::LOG_ORIGIN_UNFLAG, 
			    "**************************************\nFLAGGING HAS BEEN DISABLED TEMPORARILY DUE TO NEW INDEXER DEPLOYMENT!!!!!",
			    flag ? PMS::LOG_EVENT_FLAG : PMS::LOG_EVENT_UNFLAG);

  // Disabled 2000Jul31 (gmoellen)

  /*
  // List of flags
  Vector<Int> flagchunk(1000,-1),flagindex(1000,-1);
  
  Double thisx, thisy;
  stringstream ss;
  Int nFound = 0, n = nPoints(), flsz;
  
  for(Int i = 0; i < n; i++) {      

    // The following sets currChunk_ and irel_ (as needed below)
    getXY(i, thisx, thisy);
    
    for(uInt j = 0; j < regions.size(); j++) {
      if(thisx > regions[j].left() && thisx < regions[j].right() &&
	 thisy > regions[j].bottom() && thisy < regions[j].top()) {
	nFound++;
 
	// The following assumes currChunk_ and irel_ are properly set...
	flagInCache(flagging, flag);
        
	// Record this flags indices so we can apply to MS (VisSet) below
	flsz = flagchunk.nelements();
	if(flsz < nFound) {
	  // Add 50% more space (w/ copy!)
	  flagchunk.resize(Int(flsz * 1.5), True);
	  flagindex.resize(Int(flsz * 1.5), True);
	}
	flagchunk(nFound - 1) = currChunk_;
	flagindex(nFound - 1) = irel_;
        
	//  	reportMeta(thisx, thisy, ss);
	//  ss << '\n';
      }
    }
  }
  
  // Refresh the plot mask to reflect newly flagged data
  for(Int ichk = 0; ichk < nChunk(); ichk++)
    setPlotMask(ichk);
  
  // cout << "Finished in-memory flagging." << endl;
  
  
  // shrink flag list to correct size
  if(flagchunk.nelements() > uInt(nFound)) {
    flagchunk.resize(nFound, True);
    flagindex.resize(nFound, True);
  }
  
  // cout << "&VisSet = " << flagging.getVisSet() << endl;
  
  
  // Set the flags in the MS
//  flagInVisSet(flagging, flagchunk, flagindex, flag);
  
  ss << (flag ? "FLAGGED " : "UNFLAGGED ") << nFound 
     << " points among " << n << " in "
     << flagtimer.all_usec()/1.0e6 << "s.";
  
  return new PlotLogMessage(PMS::LOG_ORIGIN,
			    flag ? PMS::LOG_ORIGIN_FLAG : PMS::LOG_ORIGIN_UNFLAG, 
			    ss.str(),
			    flag ? PMS::LOG_EVENT_FLAG : PMS::LOG_EVENT_UNFLAG);
  */

}

  /*

// Disabled 2009Jul31 (gmoellen)
void PlotMSCacheIndexer::flagInCache(const PlotMSFlagging& flagging,Bool flag) {

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
    corr=Slice(0,chunkShapes()(0,currChunk_),1);
   
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
    chan=Slice(0,chunkShapes()(1,currChunk_),1);
  

  // Set Flag range on baseline axis:
  Int ibsln(-1);
  if (netAxesMask_(2)) {
    // specific correlation
    ibsln=(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_);
    bsln=Slice(ibsln,1,1);
  } 
  else 
    // All baselines
    bsln=Slice(0,chunkShapes()(2,currChunk_),1);
  
  // Set the sliced flag
  Cube<Bool> flagcube(plotmscache_->flag(currChunk_));
  flagcube(corr,chan,bsln)=flag;

}
  */

void PlotMSCacheIndexer::collapseMask0000(Int ch,Array<Bool>& collmask) {
  collmask.resize(IPosition(1,1));
  collmask(IPosition(1,0)) = (ntrue(*plmask_[ch]) > uInt(0));
}

void PlotMSCacheIndexer::collapseMask1000(Int ch,Array<Bool>& collmask) {
  collmask.resize();
  // collapse on chan, row
  collmask = operator>(partialNTrue(*plmask_[ch],IPosition(2,1,2)),uInt(0));
}

void PlotMSCacheIndexer::collapseMask0100(Int ch,Array<Bool>& collmask) {
  collmask.resize();
  // collapse on corr, row
  collmask = operator>(partialNTrue(*plmask_[ch],IPosition(2,0,2)),uInt(0));
}

void PlotMSCacheIndexer::collapseMask0010(Int ch,Array<Bool>& collmask) {
  collmask.resize();
  // collapse on corr, chan
  collmask = operator>(partialNTrue(*plmask_[ch],IPosition(2,0,1)),uInt(0));
}

void PlotMSCacheIndexer::collapseMask0110(Int ch,Array<Bool>& collmask) {
  collmask.resize();
  // collapse on corr
  collmask = operator>(partialNTrue(*plmask_[ch],IPosition(1,0)),uInt(0));
}

void PlotMSCacheIndexer::collapseMask1110(Int ch,Array<Bool>& collmask) {
  // just reference plmask directly
  collmask.reference(*plmask_[ch]);
}

void PlotMSCacheIndexer::collapseMask0001(Int ch,Array<Bool>& collmask) {
  //  TBD: generate antenna-based mask from baseline-based flags
  IPosition nsh(3,1,1,iantmax_(ch));
  collmask.resize(nsh);
  collmask.set(True);   
}


void PlotMSCacheIndexer::computeRanges() {

  // Initialize limits
  xmin_=ymin_=xflmin_=yflmin_=DBL_MAX;
  xmax_=ymax_=xflmax_=yflmax_=-DBL_MAX;

  // Loop over chunks:
  for (Int ich=0;ich<nChunk();++ich) {
    currChunk_=ich;

    // Loop over values on each axis
    Array<Bool> xcollmask;
    (this->*collapseXMask_)(ich,xcollmask);
    Int nx=xcollmask.nelements();

    for (Int ix=0;ix<nx;++ix) {
      Double thisx=getX(currChunk_,ix);
      if (*(xcollmask.data()+ix)) {
	xmin_=min(xmin_,thisx);
	xmax_=max(xmax_,thisx);
      }
      else {
	xflmin_=min(xflmin_,thisx);
	xflmax_=max(xflmax_,thisx);
      }
    }

    Array<Bool> ycollmask;
    (this->*collapseYMask_)(ich,ycollmask);
    Int ny=ycollmask.nelements();
    for (Int iy=0;iy<ny;++iy) {
      Double thisy=getY(currChunk_,iy);
      if ( *(ycollmask.data()+iy) ) {
	ymin_=min(ymin_,thisy);
	ymax_=max(ymax_,thisy);
      }
      else {
	yflmin_=min(yflmin_,thisy);
	yflmax_=max(yflmax_,thisy);
      }
    }
  }

  cout << "Nominal Ranges: " << xmin_ << "-" << xmax_ << " / " << ymin_ << "-" << ymax_ << endl;

  // TBD:
  //  Reconcile unadjusted limits (e.g., if no flagged data found)
  //  Handle reversed axes?  (e.g., U)
  //  Handle equal-scale axes (e.g., V vs. U, real vs. imag)
  //  Handle margin padding?

}

void PlotMSCacheIndexer::log(const String& method, const String& message,
        int eventType) {

  cout << method << ": " << message << endl;

  //  plotms_->getLogger()->postMessage(PMS::LOG_ORIGIN, method, message, eventType);}

}

}


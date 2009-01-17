//# PMSMemCache.cc: Data cache for plotms.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#include <msvis/PlotMS/PMSMemCache.h>

#include <msvis/MSVis/VisBuffer.h>
#include <casa/Quanta/MVTime.h>

namespace casa {

const PMS::Axis PMSMemCache::METADATA[] = {PMS::TIME, PMS::TIME_INTERVAL,
                                           PMS::FIELD, PMS::SPW, PMS::SCAN,
                                           PMS::ANTENNA1, PMS::ANTENNA2,
                                           PMS::FLAG};
const unsigned int PMSMemCache::N_METADATA = 8;
                                           
PMSMemCache::PMSMemCache():
  nChunk_(0),
  nPoints_(),
  minX_(0),
  maxX_(0),
  minY_(0),
  maxY_(0),
  time_(),
  timeIntr_(),
  field_(),
  spw_(),
  scan_(),
  xdiv_(1),
  xmod_(INT_MAX),
  dataLoaded_(false),
  currentSet_(false)
{}

PMSMemCache::~PMSMemCache() {
  // Deflate everything
  deleteCache();
}

// increase the number of chunks we can store
void PMSMemCache::increaseChunks(Int nc) {

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
  time_.resize(nChunk_,True);
  timeIntr_.resize(nChunk_,True);
  field_.resize(nChunk_,True);
  spw_.resize(nChunk_,True);
  scan_.resize(nChunk_,True);

  antenna1_.resize(nChunk_,False,True);
  antenna2_.resize(nChunk_,False,True);
  freq_.resize(nChunk_,False,True);
  amp_.resize(nChunk_,False,True);
  pha_.resize(nChunk_,False,True);
  flag_.resize(nChunk_,False,True);
  flagrow_.resize(nChunk_,False,True);
  
  // Construct (empty) pointed-to Vectors/Arrays
  for (Int ic=oldnChunk;ic<nChunk_;++ic) {
    antenna1_[ic] = new Vector<Int>();
    antenna2_[ic] = new Vector<Int>();
    freq_[ic] = new Vector<Double>();
    amp_[ic] = new Array<Float>();
    pha_[ic] = new Array<Float>();
    flag_[ic] = new Array<Bool>();
    flagrow_[ic] = new Vector<Bool>();
  }

  // Zero the cumulative count
  nPoints_.resize(nChunk_);
  nPoints_=0;


}

// fill cache per VisBuffer
void PMSMemCache::append(const VisBuffer& vb, Int vbnum, PMS::Axis xAxis,
        PMS::Axis yAxis) {

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
    
  if (vbnum > nChunk_-1) increaseChunks();

  // Fill the required portion of the cache
  for(unsigned int i = 0; i < N_METADATA; i++)
      loadAxis(vb, vbnum, METADATA[i]);

  // Fill optional portions of the cache (will need some enum logic here)
  loadAxis(vb, vbnum, PMS::FREQUENCY); // eventually: xAxis and yAxis if not
                                       // already in cache due to metadata
  //  *Um_[vbnum] = vb.uvw()(---Slice---);
  //  *Vm_[vbnum] = vb.uvw()(---Slice---);
  //  *Wm_[vbnum] = vb.uvw()(---Slice---);

  // Fill DATA, e.g. amplitude, phase
  //  TBD: handle data/model/corrected choices (pending scratch column revision?)
  loadAxis(vb, vbnum, PMS::AMP); // eventually: xAxis and yAxis if not already
                                 // in cache due to metadata

  // The following version makes the amp_ arrays 1D -- do not use, for now, since
  //   other code needs to know the multi-D shape of amp_.
  //  *amp_[vbnum] = amplitude(vb.visCube()).reform(IPosition(1,vb.visCube().nelements()));

  //  If we want phase, do this:
  //  *pha_[vbnum] = phase(vb.visCube());

  // Flags (used as masks for plotting data)
  //   TBD: need collapsed versions of flags, e.g. for time
  // now done in metadata above: *flag_[vbnum] = vb.flagCube();

  // Don't bother with flagrow yet
  //  *flagrow_[vbnum] = vb.flagRow()

  // update which axes are loaded in the cache, including metadata
  loadedAxes_.resize(N_METADATA + 2); // eventually, calculated using xAxis and
                                      // yAxis
  unsigned int i = 0;
  for(; i < N_METADATA; i++) loadedAxes_[i] = METADATA[i];
  loadedAxes_[i++] = PMS::FREQUENCY; // eventually, xAxis and yAxis if not
  loadedAxes_[i++] = PMS::AMP;       // metadata
  dataLoaded_ = true;
}

bool PMSMemCache::readyForPlotting() const {
    return dataLoaded_ && currentSet_;
}

void PMSMemCache::setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis) {

  // WARNING: Hardwired to setup amp vs freq plot
  
  // WARNING: Assumes nCorr, nChan uniform over chunks (not true for some multi-spw data):
  xdiv_=amp_[0]->shape()(0);   // amp_ must be 3D!
  xmod_=freq_[0]->shape()(0);

  //cout << "xdiv_ = " << xdiv_ << endl;
  //cout << "xmod_ = " << xmod_ << endl;

  // Count up the total number of points we will plot
  //   (keep a cumualtive running total)
  Int cumulativeN(0);
  for (Int ic=0;ic<nChunk_;++ic) {
    cumulativeN+=amp_[ic]->nelements();
    nPoints_(ic)=cumulativeN;
  }

  //cout << "nPoints_ = " << nPoints_ << endl;

  currentX_ = xAxis; currentY_ = yAxis;
  currentSet_ = true;
  computeRanges();
}


Double PMSMemCache::getX(Int i) {

  // WARNING: Hardwired to get freq!

  // Find correct chunk and index offset
  setChunk(i);

  // Offset the index
  if (currChunk_>0) i-=(nPoints_(currChunk_-1));

  // Manually offset the pointer:
  return Double( *(freq_[currChunk_]->data()+(i/xdiv_)%xmod_) );

  // Dereference the Vector, and use ordinary Vector indexing:
  //  return Double( (*freq_[currChunk_])((i/xdiv_)%xmod_) );
		
}

Double PMSMemCache::getY(Int i) {

  // WARNING: Hardwired to get amp!

  // Find correct chunk and index offset
  setChunk(i);

  // Offset the index
  if (currChunk_>0) i-=(nPoints_(currChunk_-1));

  // Manually offset the pointer:
  return Double( *(amp_[currChunk_]->data()+i) );

  // Dereference the Vector, and use ordinary Vector indexing:
  // return Double( (*amp_[ic])(IPosition(1,i)) );
  
}

void PMSMemCache::getXY(Int i,Double& x,Double& y) {

  // WARNING: Hardwired to get freqamp!

  // Find correct chunk and index offset
  setChunk(i);

  // Offset the index
  if (currChunk_>0) i-=(nPoints_(currChunk_-1));

  x= Double(*(freq_[currChunk_]->data()+(i/xdiv_)%xmod_));
  y= Double(*(amp_[currChunk_]->data()+i));

}



void PMSMemCache::getRanges(Double& minX, Double& maxX, Double& minY,
        Double& maxY) {
    if(!currentSet_) return;
    minX = minX_; maxX = maxX_;
    minY = minY_; maxY = maxY_;
}


void PMSMemCache::locateNearest(Double x, Double y) {
  
  // WARNING: Hardwired to for amp vs freq plot

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

  reportMeta(minic,minidx);

}

void PMSMemCache::locateRange(Double xmin,Double xmax,Double ymin,Double ymax) {

  // WARNING: Hardwired to for amp vs freq plot

  Double *thisx(NULL);
  Float *thisy(NULL);

  for (Int ic=0;ic<nChunk_;++ic) {
    Int npts=amp_[ic]->nelements();
    if (npts>0) {
      for (Int i=0;i<npts;++i) {

	thisx=freq_[ic]->data()+(i/xdiv_)%xmod_;
	thisy=amp_[ic]->data()+i;

	if (*thisx > xmin && *thisx < xmax &&
	    *thisy > ymin && *thisy < ymax) {
	  reportMeta(ic,i);
	}
      }

    }
  }

}

vector<pair<PMS::Axis, unsigned int> > PMSMemCache::loadedAxes() const {
    vector<pair<PMS::Axis, unsigned int> > v(N_METADATA + loadedAxes_.size());
    
    unsigned int i = 0;
    for(; i < N_METADATA; i++) v[i].first = METADATA[i];
    for(; i < v.size(); i++) v[i].first = loadedAxes_[i];
    
    for(i = 0; i < v.size(); i++) v[i].second = nPointsForAxis(v[i].first);
    
    return v;
}

void PMSMemCache::reportMeta(Int ic,Int i) {

  // We already know the chunk
  currChunk_=ic;

  // offset the index so getX/getY work correctly below
  Int offset(0);
  if (currChunk_>0)
    offset=nPoints_(currChunk_-1);
  
  cout << "i = " << i+offset << " ";
  cout << "Scan=" << scan_(ic) << " ";
  cout << "Field=" << field_(ic) << " ";
  cout << "Time=" << MVTime(time_(ic)/C::day).string(MVTime::YMD,7) << " ";
  cout << "BL=" << *(antenna1_[ic]->data()+(i/xdiv_/xmod_)) << "-"
       << *(antenna2_[ic]->data()+(i/xdiv_/xmod_)) << " ";
  cout << "Spw=" << spw_(ic) << " ";
  cout << "Chan=" << (i/xdiv_)%xmod_ << " ";
  cout << "Corr=" << (i%xdiv_) << " ";
  cout << "Freq=" << getX(i+offset) << " ";
  cout << "Amp="  << getY(i+offset) << " ";
  cout << endl;

}

void PMSMemCache::setChunk(Int i) {

  if (i==0) currChunk_=0;  // probably insufficient as a general reset!

  // Bump up if needed
  if (i > (nPoints_(currChunk_)-1)) ++currChunk_; 

}

void PMSMemCache::deleteCache() {
  
  // Deflate simple arrays:
  time_.resize(0);
  timeIntr_.resize(0);
  field_.resize(0);
  spw_.resize(0);
  scan_.resize(0);

  // Delete PtrBlock contents
  for (Int ic=0;ic<nChunk_;++ic) {
    if (antenna1_[ic]) delete antenna1_[ic];
    if (antenna2_[ic]) delete antenna2_[ic];
    if (freq_[ic])     delete freq_[ic];
    if (amp_[ic])      delete amp_[ic];
    if (pha_[ic])      delete pha_[ic];
    if (flag_[ic])     delete flag_[ic];
    if (flagrow_[ic])  delete flagrow_[ic];
  }
  antenna1_.resize(0);
  antenna2_.resize(0);
  freq_.resize(0);
  amp_.resize(0);
  pha_.resize(0);
  flag_.resize(0);
  flagrow_.resize(0);
  
}

void PMSMemCache::loadAxis(const VisBuffer& vb, Int vbnum, PMS::Axis axis) {
    switch(axis) {
    case PMS::TIME: // assumes time unique in VB
        time_(vbnum) = vb.time()(0); break;
        
    case PMS::UVDIST:
        break;
        
    case PMS::CHANNEL:
        break;
        
    case PMS::CORR:
        break;
        
    case PMS::FREQUENCY:
        *freq_[vbnum] = vb.frequency(); break;
        
    case PMS::VEL_RADIO:
        break;
        
    case PMS::VEL_OPTICAL:
        break;
        
    case PMS::VEL_RELATIVISTIC:
        break;
        
    case PMS::U:
        break;
        
    case PMS::V:
        break;
        
    case PMS::W:
        break;
        
    case PMS::AZIMUTH:
        break;
        
    case PMS::ELEVATION:
        break;
        
    case PMS::BASELINE:
        break;
        
    case PMS::HOURANGLE:
        break;
        
    case PMS::PARALLACTICANGLE:
        break;
        
    case PMS::AMP:
        *amp_[vbnum] = amplitude(vb.visCube()); break;
        
    case PMS::PHASE:
        break;
        
    case PMS::REAL:
        break;
        
    case PMS::IMAG:
        break;
        
    case PMS::WEIGHT:
        break;
        
    case PMS::ANTENNA1:
        *antenna1_[vbnum] = vb.antenna1(); break;
        
    case PMS::ANTENNA2:
        *antenna2_[vbnum] = vb.antenna2(); break;
        
    case PMS::TIME_INTERVAL: // assumes timeInterval unique in VB
        timeIntr_(vbnum) = vb.timeInterval()(0); break;
        
    case PMS::FIELD:
        field_(vbnum) = vb.fieldId(); break;
        
    case PMS::SCAN: // assumes scan unique in VB
        scan_(vbnum) = vb.scan()(0); break;
        
    case PMS::SPW:
        spw_(vbnum) = vb.spectralWindow(); break;
        
    case PMS::FLAG:
        *flag_[vbnum] = vb.flagCube(); break;
        
    case PMS::FLAG_ROW:
        break;
    }
}

unsigned int PMSMemCache::nPointsForAxis(PMS::Axis axis) const {
    switch(axis) {
    case PMS::TIME: return time_.size();
    case PMS::FREQUENCY: case PMS::AMP: case PMS::PHASE: case PMS::ANTENNA1:
    case PMS::ANTENNA2: case PMS::FLAG: case PMS::FLAG_ROW: {
        unsigned int n = 0;
        for(unsigned int i = 0; i < freq_.size(); i++) {
            if(axis == PMS::FREQUENCY)     n += freq_[i]->size();
            else if(axis == PMS::AMP)      n += amp_[i]->size();
            else if(axis == PMS::PHASE)    n += pha_[i]->size();
            else if(axis == PMS::ANTENNA1) n += antenna1_[i]->size();
            else if(axis == PMS::ANTENNA2) n += antenna2_[i]->size();
            else if(axis == PMS::FLAG)     n += flag_[i]->size();
            else if(axis == PMS::FLAG_ROW) n += flagrow_[i]->size();
        }
        return n;
    }     
    case PMS::TIME_INTERVAL: return timeIntr_.size();    
    case PMS::FIELD: return field_.size();      
    case PMS::SCAN: return scan_.size();     
    case PMS::SPW: return spw_.size();     
        
    default: return 0;
    }
}

void PMSMemCache::computeRanges() {
    // WARNING: Hardwired to for amp vs freq plot

    minX_=DBL_MAX;
    maxX_=-DBL_MAX;
    minY_=DBL_MAX;
    maxY_=-DBL_MAX;

    for (Int ic=0;ic<nChunk_;++ic) {
        Int npts=amp_[ic]->nelements();
        if (npts>0) {

            IPosition ip=amp_[ic]->shape();
            Int ncor(ip(0)),nchan(ip(1)),nrow(ip(2));

            Float *a=amp_[ic]->data();
            Double *f0=freq_[ic]->data();
            Double *f=f0;
            for (Int irow=0;irow<nrow;++irow) {
                f=f0;
                for (Int ichan=0;ichan<nchan;++ichan,++f) {
                    for (Int icor=0;icor<ncor;++icor,++a) {
                        minX_=min(minX_,*f);  // this is redundant here!
                        maxX_=max(maxX_,*f);
                        minY_=min(minY_,*a);
                        //      minY_=min(minY_,amp_[ic]->operator()(IPosition(3,icor,ichan,irow)));
                        maxY_=max(maxY_,*a);
                    }
                }
            }
        }
    }
    
    //cout << "Ranges: dX=" << minX_ << "-" << maxX_ << " dY=" << minY_ << "-" << maxY_ << endl;
}

}

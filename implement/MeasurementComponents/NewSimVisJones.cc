//# NewSimVisJones.h: Definition for SimVisJones matrices
//# Copyright (C) 1996,1997,1999,2000,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#include <synthesis/MeasurementComponents/NewSimVisJones.h>

#include <casa/BasicMath/Random.h>
#include <casa/Arrays/ArrayPosIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/Arrays/Slice.h>
#include <casa/Utilities/Sort.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSFeedIndex.h>
#include <calibration/CalTables/SolvableVJMRec.h>
#include <calibration/CalTables/SolvableVJDesc.h>
#include <calibration/CalTables/CalTable.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// create and initiallize a Jones matrix with no noise added.
NewSimVisJones::NewSimVisJones(Int typeseed, Int seed) : VisJones(), 
    typeseed_p(typeseed), rand_p(seed, typeseed), dist_p() { }

// create a Jones matrix for a given number of antennae and feeds.
NewSimVisJones::NewSimVisJones(uInt nants, uInt nfeeds, uInt nspw, 
			 Int typeseed, Int seed) : 
    VisJones(), typeseed_p(typeseed), rand_p(seed, typeseed), dist_p() 
{
    resizeNoiseConfig(nants, nfeeds, nspw);
}

// Create a copy of another SimVisJones.
NewSimVisJones::NewSimVisJones(const NewSimVisJones& other) : 
    typeseed_p(other.typeseed_p), rand_p(other.rand_p.seed1(), other.typeseed_p)
{
    *this = other;
}

// Assignment
NewSimVisJones& NewSimVisJones::operator=(const NewSimVisJones &other) {
    dist_p.resize(other.dist_p.nrow(), other.dist_p.ncolumn(), 
		  other.dist_p.nplane());
    dist_p = other.dist_p;
    return *this;
}

NewSimVisJones::~NewSimVisJones() {
}

// set the parameters for the random component of the gain variation.
//   antenna is the (zero-based) index of the antenna set the noise for
//     or a negative value to set the noise for all antennae.
//   receptor is the (zero-based) index of the receptor to set the noise
//     for or negative to set all receptors simultaneously; since only 
//     2x2 Jones matrices are currently supported, this should be less 
//     than 2.
//   comp is either SimVisJones::amp or SimVisJones::phase to indicate 
//     which component of the gain noise should be applied to.
//   width is the full characteristic width of the distribution.  If the 
//     distribution is normal, the width is twice the root-mean-square of 
//     the distribution (i.e. 2*sigma).  
//   feed is the (zero-based) index of the receptor to set the noise for
//     or negative to set all the feeds simultaneously.
//   spwin is the (zero-based) ID of the spectral window to set the noise
//     for or negative to set all windows simultaneously.
//   type is the type of noise distribution as one of the identifiers 
//     given in the Distribution enumeration (ideal, normal, or uniform); 
//     use ideal to prevent a random component from being added to the 
//     gains.  
void NewSimVisJones::setNoise(Int antenna, Int receptor, Component comp, 
			   Double width, Int feed, Int spwin, 
			   Distribution type) 
{
    if (width == 0) type = ideal;
    if (width < 0) width *= -1;

    uInt a=0, na=dist_p.nrow();
    uInt f=0, nf=dist_p.ncolumn();
    uInt w=0, nw=dist_p.nplane();
    uInt r=0, nr=2;
    if (a >= 0) a = na = antenna;
    if (f >= 0) f = nf = feed;
    if (w >= 0) w = nw = spwin;
    if (r >= 0) r = nr = receptor;
	
    for(; a < na; a++) {
      for(; f < nf; f++) {
	for(; w < nw; w++) {
	  for(; r < nr; r++) {
	     if (type == normal) {
		dist_p(a,f,w)(r,comp) = new Normal(&rand_p, width*width/4);
	     }
	     else if (type == uniform) {
		dist_p(a,f,w)(r,comp) = new Uniform(&rand_p, -width/2, width/2);
	     }
	  }
	}
      }
    }
}

// resize the noise configuration containers.  Obviously, this should
// be called whenever the number of antennae or feeds change.
void NewSimVisJones::resizeNoiseConfig(uInt nants, uInt nfeeds, uInt nspw) {
    uInt nr = dist_p.nrow(), nc = dist_p.ncolumn(), nw = dist_p.nplane();
    if (nr == nants && nc == nfeeds && nw == nspw) return;

    Cube<Matrix<CountedPtr<Random> > > tmp;
    if (dist_p.nrow() > 0 && dist_p.ncolumn() > 0 && dist_p.nplane() > 0) {
	// copy distributions
	tmp = dist_p;
    }

    dist_p.resize(nants, nfeeds, nspw);

    // copy in previously set distributions.  This is not done if dist_p
    // originally had (0,0) size.
    uInt a=0, f=0, w=0;
    Matrix<CountedPtr<Random> > ideal(2, 2);
    for(a=0; a < nr && a < nants; a++) {
	for(f=0; f < nc && f < nfeeds; f++) {
	    for(w=0; w < nw && w < nspw; w++) {
		dist_p(a, f, w) = tmp(a, f, w);
	    }
	    for(; w < nspw; w++) {
		dist_p(a, f, w) = ideal;
	    }
	}
	for(; f < nfeeds; f++) {
	    for(w=0; w < nspw; w++) {
		dist_p(a, f, w) = ideal;
	    }
	}
    }
    for(; a < nants; a++) {
	for(f=0; f < nfeeds; f++) {
	    for(w=0; w < nspw; w++) {
		dist_p(a, f, w) = ideal;
	    }
	}
    }
}

// add the noise for a given antenna and feed to the gains in the given 
// jones matrix.
//   gains is a 2x2 matrix containing actual gains (presumably for the 
//     antenna and feed specified).  Each row contains a gain for one 
//     polarization; the columns give the amplitude and phase of the 
//     gains. 
//   antenna is the (zero-based) index of the antenna to apply
//     the noise for.
//   feed is the (zero-based) index of the receptor to apply
//     the noise for.
Bool NewSimVisJones::applyNoise(SquareMatrix<Float, 2> &gains, uInt antenna, 
			     uInt feed, uInt spwid) const 
{
    Matrix<CountedPtr<Random> > distm = dist_p(antenna, feed, spwid);
    if (! distm(0,0).null()) gains(0,0) = abs(gains(0,0) + (*(distm(0,0)))());
    if (! distm(1,0).null()) gains(1,0) = abs(gains(0,0) + (*(distm(1,0)))());
    if (! distm(0,1).null()) gains(0,1) += (*(distm(1,0)))();
    if (! distm(1,1).null()) gains(1,1) += (*(distm(1,1)))();
    return True;
}

// Determine if polarization re-sequencing is required
Bool NewSimVisJones::polznSwitch(const VisBuffer& vb)
{
  Vector<Int> corr=vb.corrType();
  Bool needToSwitch=False;
  if (corr.nelements() > 2) {
    needToSwitch=(corr(0)==Stokes::XX && corr(1)==Stokes::YY) ||
      (corr(0)==Stokes::RR && corr(1)==Stokes::LL);
  };
  return needToSwitch;
};

// Re-sequence to (XX,XY,YX,YY) or (RR,RL,LR,LL)
void NewSimVisJones::polznMap(CStokesVector& vis)
{
  Complex vswap=vis(1);
  vis(1)=vis(2);
  vis(2)=vis(3);
  vis(3)=vswap;
};

// Re-sequence to (XX,YY,XY,YX) or (RR,LL,RL,LR)
void NewSimVisJones::polznUnmap(CStokesVector& vis)
{
  Complex vswap=vis(3);
  vis(3)=vis(2);
  vis(2)=vis(1);
  vis(1)=vswap;
};

//******************************************

void SimAntGainCache::clear() {
    Bool dodel=False;
    OrderedMap<Double, mjJones2*>** storage = cache_p.getStorage(dodel);
    for(uInt i=0; i < cache_p.nelements(); i++) {
	if (storage[i]) {
	    clearMap(*(storage[i]));
	    delete storage[i];
	    storage[i] = 0;
	}
    }
    OrderedMap<Double, mjJones2*>* const * use = storage;
    cache_p.freeStorage(use, dodel);
}

//******************************************

// create and initiallize a Jones matrix with no noise added.
NewSimTimeDepVisJones::NewSimTimeDepVisJones(Int typeseed, Int seed) : 
    NewSimVisJones(typeseed, seed), log_p(LogOrigin("SimGJones")), 
    timedep_p(), tdi_p(5,0,0,0,0,0), antGainCache_p(), 
    intGainCache_p(), intInvGainCache_p(), igi_p(4,0,0,0,0), 
    intCacheTime_p(-1), reftime_p(0), slotintv_p(0)
{ }

// create a Jones matrix for a given number of antennae and feeds.
NewSimTimeDepVisJones::NewSimTimeDepVisJones(uInt nants, uInt nfeeds, uInt nspw, 
				       Double reftime, Int typeseed, Int seed)
    : NewSimVisJones(nants, nfeeds, nspw, typeseed, seed), 
      log_p(LogOrigin("SimGJones")), timedep_p(), tdi_p(5,0,0,0,0,0), 
      antGainCache_p(), intGainCache_p(), intInvGainCache_p(), 
      igi_p(4,0,0,0,0), intCacheTime_p(-1), reftime_p(reftime), slotintv_p(0)
{ 
    setShape(nants, nfeeds, nspw);
}

// Construct a Jones matrix ready to apply unity gains to a given 
// VisibilitySet.  
NewSimTimeDepVisJones::NewSimTimeDepVisJones(VisSet& vs, Int typeseed, Int seed, 
				       Double slotintv) :
    NewSimVisJones(typeseed, seed), log_p(LogOrigin("SimGJones")), 
    timedep_p(), tdi_p(5,0,0,0,0,0), antGainCache_p(), 
    intGainCache_p(), intInvGainCache_p(), igi_p(4,0,0,0,0), 
    intCacheTime_p(-1), reftime_p(0), slotintv_p(slotintv)
{ 
    initFromVisSet(vs, slotintv);
}

void NewSimTimeDepVisJones::initFromVisSet(VisSet& vs, Double slotintv) {
    uInt nfeeds = numberFeeds(vs);
    uInt nants = vs.numberAnt();
    uInt nspw = vs.numberSpw();
    setShape(nants, nfeeds, nspw);
    
//    if (slotintv <= 0) slotintv = DBL_MAX;
    Block<Int> columns(0);
    VisSet tmpvs(vs, columns, 3600);

    VisIter &ioc = tmpvs.iter();
    Double mnt, mxt;
    reftime_p = DBL_MAX;
    Double endtime = 0;
    Vector<Double> time;
    for (ioc.originChunks(); ioc.moreChunks(); ioc.nextChunk()) {
	ioc.time(time);
	minMax(mnt, mxt, time);
	if (mnt < reftime_p) reftime_p = mnt;
	if (mxt > endtime) endtime = mxt;
    }

}

// Create a copy of another SimVisJones.
NewSimTimeDepVisJones::NewSimTimeDepVisJones(const NewSimTimeDepVisJones& other) : 
    NewSimVisJones(other), timedep_p(other.timedep_p), tdi_p(other.tdi_p),
    antGainCache_p(other.getAntennaCount(), other.getFeedCount(), 
		   other.getSpWinCount()),
    intGainCache_p(other.intGainCache_p.shape()), 
    intInvGainCache_p(other.intInvGainCache_p.shape()), igi_p(other.igi_p),
    intCacheTime_p(-1), slotintv_p(other.slotintv_p)
{ }

NewSimTimeDepVisJones& 
NewSimTimeDepVisJones::operator=(const NewSimTimeDepVisJones &other) 
{
    NewSimVisJones::operator=(other);
    timedep_p.resize(other.timedep_p.shape());
    timedep_p = other.timedep_p;
    antGainCache_p.resize(other.getAntennaCount(), other.getFeedCount(), 
			  other.getSpWinCount());
    intGainCache_p.resize(other.intGainCache_p.shape()); 
    intInvGainCache_p.resize(other.intInvGainCache_p.shape());
    intCacheTime_p = -1;
    slotintv_p = other.slotintv_p;
    clearCache();
    return *this;
}

NewSimTimeDepVisJones::~NewSimTimeDepVisJones() {
}

void NewSimTimeDepVisJones::setTimeDep(Int antenna, Int receptor, Component comp, 
				    const Function1D<Float>& func, 
				    Int feed, Int spw) 
{
    CountedPtr<Function1D<Float> > 
	ptr(dynamic_cast<Function1D<Float>*>(func.clone()));

    uInt sa=0, sf=0, sw=0, sr=0, nr=2, 
	na=getAntennaCount(), nf=getFeedCount(), nw=getSpWinCount();
    if (antenna  >= 0) sa = na = antenna;
    if (feed     >= 0) sf = nf = feed;
    if (spw      >= 0) sw = nw = spw;
    if (receptor >= 0) sr = nr = receptor;

    tdi_p(4) = comp;
    for(uInt a=sa; a < na; a++) {
	tdi_p(0) = a;
	for(uInt f=sf; f < nf; f++) {
	    tdi_p(1) = f;
	    for(uInt w=sw; w < nw; w++) {
		tdi_p(2) = w;
		for(uInt r=sr; r < nr; r++) {
		    tdi_p(3) = r;
		    timedep_p(tdi_p) = ptr;
		}
		invalidateCache(a, f, w);
	    }
	}
    }
}

void NewSimTimeDepVisJones::removeTimeDep(Int antenna, Int receptor, 
				       Component comp, Int feed, Int spw) 
{
    CountedPtr<Function1D<Float> > ptr;

    uInt a=0, f=0, w=0, r=0, nr=2, 
	na=getAntennaCount(), nf=getFeedCount(), nw=getSpWinCount();
    if (a >= 0) a = na = antenna;
    if (f >= 0) f = nf = feed;
    if (w >= 0) w = nw = spw;
    if (r >= 0) r = nr = receptor;

    tdi_p(4) = comp;
    for(; a < na; a++) {
	tdi_p(0) = a;
	for(; f < nf; f++) {
	    tdi_p(1) = f;
	    for(; w < nw; w++) {
		tdi_p(2) = w;
		for(; r < nr; r++) {
		    tdi_p(3) = r;
		    timedep_p(tdi_p) = ptr;
		}
		invalidateCache(a, f, w);
	    }
	}
    }
}

void NewSimTimeDepVisJones::clearCache() {
    uInt na=getAntennaCount(), nf=getFeedCount(), nw=getSpWinCount();
    uInt a, f, w;
    for(a=0; a < na; a++) {
	for(f=0; f < nf; f++) {
	    for(w=0; w < nw; w++) {
		invalidateCache(a, f, w);
	    }
	}
    }
}

const mjJones4& NewSimTimeDepVisJones::getIntGain(Int ant1, Int feed1, 
					       Int ant2, Int feed2,
					       Int spw, Double time)
{
    checkIntCacheTime(time);
    CountedPtr<mjJones4> &jptr = 
	intGainCache_p(setigi(ant1, ant2, feed1, feed2));
    if (! jptr.null()) return *jptr;

    jptr = new mjJones4();
    mjJones4& jout = *jptr;

    // adjust time: normalize to the start time; if we're binning 
    // the gains into slots, change time to the middle of a slot 
    // interval.
    time -= reftime_p;
    if (slotintv_p > 0) 
	time = slotintv_p/2.0 + floor(time/slotintv_p)*slotintv_p;

    if (! antGainCache_p.isDefined(ant1, feed1, spw, time))
	cacheAntGain(ant1, feed1, spw, time);
    if (! antGainCache_p.isDefined(ant2, feed2, spw, time))
	cacheAntGain(ant2, feed2, spw, time);
    const mjJones2& j1 = antGainCache_p(ant1, feed1, spw, time);
    const mjJones2& j2 = antGainCache_p(ant2, feed2, spw, time);

    mjJones2 jconj = j2;
    jconj.conj();
    directProduct(jout, j1, jconj);

    return jout;
}

const mjJones4& NewSimTimeDepVisJones::getIntInvGain(Int ant1, Int feed1, 
						  Int ant2, Int feed2,
						  Int spw, Double time)
{
    checkIntCacheTime(time);
    CountedPtr<mjJones4> &jptr = 
	intInvGainCache_p(setigi(ant1, ant2, feed1, feed2));
    if (! jptr.null()) return *jptr;

    jptr = new mjJones4();
    mjJones4& jout = *jptr;

    // Catch the un-invertable matrix
    Matrix<Complex> jtmp = 
	invert(getIntGain(ant1,ant2,feed1,feed2, spw, time).matrix());
    if (jtmp.nelements() == 0) {
	jout = Complex(1.0, 0.0);
    } else {
	jout = jtmp;
    }

    return jout;
}

VisBuffer& NewSimTimeDepVisJones::apply(VisBuffer& vb) {
    Int ant1, ant2, feed1, feed2, chn;
    Double time;
    Bool polSwitch=polznSwitch(vb);
    Int spw=vb.spectralWindow();

    for (Int row=0; row < vb.nRow(); row++) {
	if (! vb.flagRow()(row)) {
	    ant1 = vb.antenna1()(row);
	    ant2 = vb.antenna2()(row);
	    feed1 = vb.msColumns().feed1()(row);
	    feed2 = vb.msColumns().feed2()(row);
	    time = vb.time()(row);

	    const mjJones4& j = getIntGain(ant1,feed1, ant2,feed2, spw, time);
	    for(chn=0; chn < vb.nChannel(); chn++) {
		if (! vb.flag()(chn, row)) {

		    // apply Jones matrix to each channel
		    if (polSwitch) polznMap(vb.visibility()(chn, row));
		    vb.visibility()(chn, row) *= j;
		    if (polSwitch) polznUnmap(vb.visibility()(chn,row));
		}
	    }
	}
    }

    return vb;
}

VisBuffer& NewSimTimeDepVisJones::applyInv(VisBuffer& vb) {
    Int ant1, ant2, feed1, feed2, chn;
    Double time;
    Bool polSwitch=polznSwitch(vb);
    Int spw=vb.spectralWindow();

    for (Int row=0; row < vb.nRow(); row++) {
	if (! vb.flagRow()(row)) {
	    ant1 = vb.antenna1()(row);
	    ant2 = vb.antenna2()(row);
	    feed1 = vb.msColumns().feed1()(row);
	    feed2 = vb.msColumns().feed2()(row);
	    time = vb.time()(row);

	    const mjJones4& j = getIntInvGain(ant1,feed1,ant2,feed2,spw,time);
	    for(chn=0; chn < vb.nChannel(); chn++) {
		if (! vb.flag()(chn, row)) {

		    // apply Jones matrix to each channel
		    if (polSwitch) polznMap(vb.visibility()(chn, row));
		    vb.visibility()(chn, row) *= j;
		    if (polSwitch) polznUnmap(vb.visibility()(chn,row));
		}
	    }
	}
    }

    return vb;
}

void NewSimTimeDepVisJones::store(const String& file, MeasurementSet &ms, 
			       Bool append)
{
    CalTable *tab = 0;
    try {
	Int nMain=0, nDesc=0, nHist=0;

	// Open the output file if it already exists and is being appended to.
	if (append && Table::isWritable (file)) {
	    tab  = new CalTable (file, Table::Update);
	    nMain = tab->nRowMain();
	    nDesc = tab->nRowDesc();
	    nHist = tab->nRowHistory();
	} else {
	    // Create a new calibration table
	    SolvableVisJonesDesc svjDesc (typeName());
	    tab = new CalTable (file, svjDesc);
	}

	String msname = ms.tableName();
	Matrix<Int> caldescid(getFeedCount(), getSpWinCount());
	caldescid = -1;

	// have some of the MS subtables at the ready.
	ROMSColumns msc(ms);
	const ROMSSpWindowColumns& spwc = msc.spectralWindow();
	const ROMSFeedColumns& fc = msc.feed();

	Table chunk;
	ScalarColumn<Int> fieldId;
	ScalarColumn<Int> arrayId;
	ScalarColumn<Int> obsId;
	ScalarColumn<Int> scanNo;
	ScalarColumn<Int> procId;
	ScalarColumn<Int> stateId;
	ScalarColumn<Int> feed1;
	ScalarColumn<Int> feed2;
	ScalarColumn<Int> ant1;
	ScalarColumn<Int> ant2;
	ScalarColumn<Double> time;
	ScalarColumn<Double> intv;
	Vector<Int> feeds, fids;
	Vector<uInt> ufi, sfi;
	Int nr, feed, spw, x;
	Double y, freq, thetime;
	Cube<Int> ids(2,1,1,0);
	Vector<Double> z;
	Matrix<Double> chfreq(1,1);
	Cube<Double> reffreq(2,1,1, 0.0);
	String s;
	IPosition ashape(4,2,2,1,1);
	Array<Complex> gain(ashape);
	IPosition gi(4,0);
	Array<Float> fit(ashape, 1.0);
	Array<Bool> fitok(ashape, True);

	SolvableVisJonesMRec crec;
	crec.defineTimeEP(0);
	crec.defineCalGrp(0);
	crec.definePhaseId(0);
	crec.definePulsarBin(0);
	crec.definePulsarGateId(0);
	crec.defineCalHistoryId(0);
	crec.defineRefAnt(ids);
	crec.defineRefFeed(ids);
	crec.defineTotalSolnOk(True);
	crec.defineTotalFit(1.0);
	crec.defineTotalFitWgt(1.0);
	crec.defineSolnOk(fitok);
	crec.defineFit(fit);
	crec.defineFitWgt(fit);

	if (! ms.lock()) 
	    throw AipsError(String("Unable to obtain lock on ") + 
			    ms.tableName());
	Block<Int> sort(4);
	sort[0] = MS::OBSERVATION_ID;
	sort[1] = MS::SCAN_NUMBER;
	sort[2] = MS::PROCESSOR_ID;
	sort[3] = MS::STATE_ID;
	MSIter mi(ms,sort,0.0);
	for(mi.origin(); mi.more(); mi++) {
	    spw = mi.spectralWindowId();
	    if (spw >= static_cast<Int>(getSpWinCount())) continue;

	    chunk = mi.table();
	    fieldId.attach(chunk, MS::columnName(MS::FIELD_ID));
	    arrayId.attach(chunk, MS::columnName(MS::ARRAY_ID));
	    obsId.attach(chunk, MS::columnName(MS::OBSERVATION_ID));
	    scanNo.attach(chunk, MS::columnName(MS::SCAN_NUMBER));
	    procId.attach(chunk, MS::columnName(MS::PROCESSOR_ID));
	    stateId.attach(chunk, MS::columnName(MS::STATE_ID));
	    feed1.attach(chunk, MS::columnName(MS::FEED1));
	    feed2.attach(chunk, MS::columnName(MS::FEED2));
	    ant1.attach(chunk, MS::columnName(MS::ANTENNA1));
	    ant2.attach(chunk, MS::columnName(MS::ANTENNA2));
	    time.attach(chunk, MS::columnName(MS::TIME));
	    intv.attach(chunk, MS::columnName(MS::INTERVAL));

	    // Get a unique list of feed ids
	    nr = chunk.nrow();
	    feeds.resize(2*nr);
	    fids.resize(nr);
//	    Slice f1sl(0, nr), f2sl(nr, nr);
	    feed1.getColumn(fids, False);
	    feeds(Slice(0,nr)) = fids;
	    feed2.getColumn(fids, False);
	    feeds(Slice(nr,nr)) = fids;
	    Sort sorter;
	    Bool dodelete;
	    Int *feeddata = feeds.getStorage(dodelete);
	    sorter.sortKey(feeddata, TpInt);
	    sorter.sort(sfi, feeds.nelements());
	    sorter.unique(ufi, sfi);

	    // set some values that will be the same for each feed
	    if (mi.newSpectralWindow()) {
		spwc.measFreqRef().get(spw, x);
		crec.defineMeasFreqRef(x);
		spwc.refFrequency().get(spw, freq);
		reffreq = freq;
		crec.defineRefFreq(reffreq);
		spwc.freqGroup().get(spw, x);
		crec.defineFreqGrp(x);
		spwc.freqGroupName().get(spw, s);
		crec.defineFreqGrpName(s);
	    }
	    crec.defineSourceName(mi.sourceName());
	    if (mi.newField()) crec.defineFieldName(mi.fieldName());

	    time.get(0, thetime);
	    crec.defineTime(thetime);

	    intv.getColumn(z, True);
	    y = max(z);
	    crec.defineInterval(y);

	    fieldId.get(0, x);
	    crec.defineFieldId(x);
	    arrayId.get(0, x);
	    crec.defineArrayId(x);
	    obsId.get(0, x);
	    crec.defineObsId(x);
	    scanNo.get(0, x);
	    crec.defineScanNo(x);
	    procId.get(0, x);
	    crec.defineProcessorId(x);
	    stateId.get(0, x);
	    crec.defineStateId(x);

	    // loop through each feed found
	    for(uInt f=0; f < ufi.nelements(); f++) {
		feed = feeds(ufi(f));
		if (feed >= static_cast<Int>(getFeedCount())) continue;

		if (caldescid(feed, spw) < 0) {

		    // write a new cal desc record
		    CalDescRecord desc;
		    desc.defineNumSpw(1);
		    desc.defineNumReceptors(getFeedCount());
		    desc.defineNJones(2);  // correct?

		    Vector<Int> nchan(1, 1);
		    desc.defineNumChan(nchan);
		    Vector<Int> spwins(1, spw);
		    desc.defineSpwId(spwins);

		    chfreq = freq;
		    desc.defineChanFreq(chfreq);
		    spwc.totalBandwidth().get(spw, freq);
		    chfreq = freq;
		    desc.defineChanWidth(chfreq);

		    spwc.numChan().get(spw, x);
		    Cube<Int> chrange(2,1,1,1);
		    chrange(1,0,0) = x-1;

		    Vector<String> poltype;
		    MSFeedIndex fidx(ms.feed());
		    ant1.get(0, x);
		    fidx.antennaId() = x;
		    fidx.feedId() = feed;
		    fidx.spectralWindowId() = spw;
		    Bool found;
		    x = fidx.getNearestRow(found);
		    fc.polarizationType().get(x, poltype);
		    desc.definePolznType(poltype);

		    desc.defineJonesType("full");
		    desc.defineMSName(msname);

		    tab->putRowDesc(nDesc, desc);
		    caldescid(feed, spw) = nDesc++;
		}

		crec.defineCalDescId(caldescid(feed, spw));
		crec.defineFeed1(feed);

		// write out records for every antenna, regardless of 
		// whether they match what's in the MS
		Int na = getAntennaCount();
		for(Int a=0; a < na; a++) {
		    crec.defineAntenna1(a);

		    const mjJones2& g = getAntGain(a, feed, spw, thetime);
		    for(gi(0)=0; gi(0) < 2; ++gi(0)) {
			for(gi(1)=0; gi(1) < 2; ++gi(1)) {
			    gain(gi) = g(gi(0),gi(1));
			}
		    }
		    crec.defineGain(gain);

		    tab->putRowMain(nMain++, crec);
		}
	    }
	}

	ms.unlock();
	delete tab;
    }
    catch (...) {
	ms.unlock();
	delete tab;
	throw;
    }
}

//******************************************
#define SJSEED_G 1
#define SJSEED_D 2
#define SJSEED_B 3

NewSimGJones::NewSimGJones(Int seed) : NewSimTimeDepVisJones(SJSEED_G, seed),
    defgains_p(2,2,0.0)
{ 
    setdefgains(1.0,0.0,1.0,0.0);
}

NewSimGJones::NewSimGJones(uInt nants, uInt nfeeds, uInt nspw, Double reftime, 
		     Int seed) 
    : NewSimTimeDepVisJones(nants, nfeeds, nspw, reftime, SJSEED_G, seed), 
      defgains_p(2,2,0.0)
{ 
    setdefgains(1.0,0.0,1.0,0.0);
}

NewSimGJones::NewSimGJones(VisSet& vs, Int seed, Double slotintv) : 
    NewSimTimeDepVisJones(vs, SJSEED_G, seed, slotintv), defgains_p(2,2,0.0)
{
    setdefgains(1.0,0.0,1.0,0.0);
}

NewSimGJones::NewSimGJones(VisSet& vs, Double amp0, Double phase0, 
		     Double amp1, Double phase1, Int seed, Double slotintv) : 
    NewSimTimeDepVisJones(vs, SJSEED_G, seed, slotintv), defgains_p(2,2,0.0)
{
    setdefgains(amp0, phase0, amp1, phase1);
}

NewSimGJones::NewSimGJones(const NewSimGJones &other) : NewSimTimeDepVisJones(other), 
    defgains_p(other.defgains_p)
{ }

NewSimGJones& NewSimGJones::operator=(const NewSimGJones &other) {
    NewSimTimeDepVisJones::operator=(other);
    defgains_p = other.defgains_p;
    return *this;
}

NewSimGJones::~NewSimGJones() {
}

mjJones2& NewSimGJones::calcAntGain(mjJones2 &jones, 
				 uInt ant, uInt feed, 
				 uInt spw, Double time) 
{
    SquareMatrix<Float, 2> gains(defgains_p);
    CountedPtr<Function1D<Float> > func;

    func = timedep_p(settdi(ant, feed, spw, 0, amp));
    if (! func.null()) gains(0,0) = (*func)(time);

    func = timedep_p(settdi(ant, feed, spw, 1, amp));
    if (! func.null()) gains(1,0) = (*func)(time);

    func = timedep_p(settdi(ant, feed, spw, 0, phase));
    if (! func.null()) gains(0,1) = (*func)(time);

    func = timedep_p(settdi(ant, feed, spw, 1, phase));
    if (! func.null()) gains(1,1) = (*func)(time);

    applyNoise(gains, ant, feed, spw);

    // this copying gymnastics is to accommodate the interface to 
    // the mjones2 (SquareMatrix) class and enables the faster matrix 
    // math.
    Vector<Complex> jdiag(2, Complex(0.0,0.0));
    jdiag(0) = polar(gains(0,0), gains(0,1));
    jdiag(1) = polar(gains(1,0), gains(1,1));
    jones = jdiag;

    return jones;
}

//******************************************

NewSimDJones::NewSimDJones(Int seed) : NewSimTimeDepVisJones(SJSEED_G, seed),
    defgains_p(2,2,0.0)
{ 
    setdefgains(1.0,0.0,1.0,0.0);
}

NewSimDJones::NewSimDJones(uInt nants, uInt nfeeds, uInt nspw, Double reftime, 
		     Int seed) 
    : NewSimTimeDepVisJones(nants, nfeeds, nspw, reftime, SJSEED_G, seed), 
      defgains_p(2,2,0.0)
{ 
    setdefgains(1.0,0.0,1.0,0.0);
}

NewSimDJones::NewSimDJones(VisSet& vs, Int seed, Double slotintv) : 
    NewSimTimeDepVisJones(vs, SJSEED_G, seed, slotintv), defgains_p(2,2,0.0)
{
    setdefgains(1.0,0.0,1.0,0.0);
}

NewSimDJones::NewSimDJones(VisSet& vs, Double amp0, Double phase0, 
		     Double amp1, Double phase1, Int seed, Double slotintv) : 
    NewSimTimeDepVisJones(vs, SJSEED_G, seed, slotintv), defgains_p(2,2,0.0)
{
    setdefgains(amp0, phase0, amp1, phase1);
}

NewSimDJones::NewSimDJones(const NewSimDJones &other) : NewSimTimeDepVisJones(other), 
    defgains_p(other.defgains_p)
{ }

NewSimDJones& NewSimDJones::operator=(const NewSimDJones &other) {
    NewSimTimeDepVisJones::operator=(other);
    defgains_p = other.defgains_p;
    return *this;
}

NewSimDJones::~NewSimDJones() {
}

mjJones2& NewSimDJones::calcAntGain(mjJones2 &jones, 
				 uInt ant, uInt feed, 
				 uInt spw, Double time) 
{
    SquareMatrix<Float, 2> gains(defgains_p);
    CountedPtr<Function1D<Float> > func;

    func = timedep_p(settdi(ant, feed, spw, 0, amp));
    if (! func.null()) gains(0,0) = (*func)(time);

    func = timedep_p(settdi(ant, feed, spw, 1, amp));
    if (! func.null()) gains(1,0) = (*func)(time);

    func = timedep_p(settdi(ant, feed, spw, 0, phase));
    if (! func.null()) gains(0,1) = (*func)(time);

    func = timedep_p(settdi(ant, feed, spw, 1, phase));
    if (! func.null()) gains(1,1) = (*func)(time);

    applyNoise(gains, ant, feed, spw);

    // this copying gymnastics is to accommodate the interface to 
    // the mjones2 (SquareMatrix) class and enables the faster matrix 
    // math.
    Matrix<Complex> joff(2,2, Complex(0.0,0.0));
    joff(0,1) = polar(gains(0,0), gains(0,1));
    joff(1,0) = polar(gains(1,0), gains(1,1));
    jones = joff;

    return jones;
}


} //# NAMESPACE CASA - END


//# NewSimVisJones.h: Definition for SimVisJones matrices
//# Copyright (C) 1996,1997,1999,2000,2002
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

#ifndef SYNTHESIS_NEWSIMVISJONES_H
#define SYNTHESIS_NEWSIMVISJONES_H
#include <synthesis/MeasurementComponents/VisJones.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <casa/BasicMath/Random.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Containers/OrderedPair.h>
#include <casa/Containers/OrderedMap.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/Logging/LogIO.h>

#include <scimath/Functionals/Function1D.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Model multiplicative gain errors for the VisEquation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
// </prerequisite>
//
// <etymology>
// SimVisJones describes random multiplicative gains to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. 
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="VisJones">VisJones</linkto>
// for how SimVisJones is to be used.
// </synopsis> 
//
// <motivation>
// The properties of an additive component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class NewSimVisJones : public VisJones {
public:

    // Supported distributions for the random component of the gain variation
    enum Distribution { 
        // a delta-function (i.e. zero-noise) distribution.  This distribution
        // type should be used when a random component is not desired.
        ideal, 
        // the normal distribution.  The random component will vary according 
        // to a normal (Gaussian) distribution.
        normal, 
        // the uniform distribution.  The random component will vary uniformily
        // over a given range.
        uniform 
    };

    // identifiers for the two components of a gain
    enum Component { amp, phase };

    // create and initiallize a Jones matrix with no noise added.
    NewSimVisJones(Int typeseed=0, Int seed=0);

    // create a Jones matrix for a given number of antennae and feeds.
    NewSimVisJones(uInt nants, uInt nfeeds, uInt nspw, Int typeseed=0, Int seed=0);

    // Create a copy of another SimVisJones.
    NewSimVisJones(const NewSimVisJones& other);

    // delete this SimVisJones
    virtual ~NewSimVisJones();

    // return the number of antennae this matrix is configured for.
    uInt getAntennaCount() const { return dist_p.nrow(); }

    // return the number of feeds this matrix is configured for.
    uInt getFeedCount() const { return dist_p.ncolumn(); }

    // return the number of feeds this matrix is configured for.
    uInt getSpWinCount() const { return dist_p.nplane(); }

    // set the number of antennae, feeds, and spectral windows this Jones
    // is configured to handle.  This needs to be called before any real 
    // calculations can be done--that is, prior to any call to apply() or
    // applyInv or any of the set...() routines.  This usually allows this
    // object to set its internal storage appropriately.  
    virtual void setShape(Int nants, Int nfeeds, Int nspwins) {
	resizeNoiseConfig(nants, nfeeds, nspwins);
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
    //     the distribution (i.e. 2*sigma).  A width of zero is equivalent
    //     type=ideal.  If comp is phase, the width is taken as a phase in
    //     radians; if comp is amp, the width is taken to be a gain magnitude.
    //   feed is the (zero-based) index of the feed to set the noise for
    //     or negative to set all the feeds simultaneously.
    //   spwin is the (zero-based) ID of the spectral window to set the noise
    //     for or negative to set all windows simultaneously.
    //   type is the type of noise distribution as one of the identifiers 
    //     given in the Distribution enumeration (ideal, normal, or uniform); 
    //     use ideal to prevent a random component from being added to the 
    //     gains.  
    virtual void setNoise(Int antenna, Int receptor, Component comp, 
			  Double width, Int feed=-1, Int spwin=-1,
			  Distribution type=normal);

    // return the type of random distribution associated with a given gain
    // component
    virtual Distribution getNoiseType(uInt antenna, uInt feed, uInt spw,
				      uInt receptor, Component comp) const 
    {
	CountedPtr<Random> ptr = dist_p(antenna, feed, spw)(receptor, comp);
	if (ptr.null()) return ideal;
	if (dynamic_cast<Normal*>(&(*ptr))) 
	    return normal;
	else if (dynamic_cast<Uniform*>(&(*ptr))) 
	    return uniform;
	else
	    return ideal;
    }

    // return the width of the randome distribution associated with a given
    // gain component.  Zero is returned if the distribution is ideal.
    virtual Double getNoiseWidth(uInt antenna, uInt feed, uInt spw, 
				 uInt receptor, Component comp) const 
    {
	CountedPtr<Random> ptr = dist_p(antenna, feed, spw)(receptor, comp);
	if (ptr.null()) return ideal;
	Normal *nd = 0;
	Uniform *ud = 0;
	if ((nd = dynamic_cast<Normal*>(&(*ptr)))) 
	    return sqrt(nd->variance())*2;
	else if ((ud = dynamic_cast<Uniform*>(&(*ptr)))) 
	    return ud->high() - ud->low();
	else
	    return 0;
    }

    // reseed the random number generator used to produce noise.  
    //   seed1 is the primary seed, and
    void reseed(Int seed) { rand_p.reseed(seed, typeseed_p); }

    // internally cache the antenna gains according to the current 
    // parameter settings.  This makes them accessible to getGains() without 
    // having to actually apply them to the MeasurmentSet.  If the gain
    // parameters change, the gain cache will be appropriately invalidated.  
    // vi is the vis iterator associated with the MeasreumentSent; it will
    // be set to origin and then advanced to the end of the set.  
//    virtual void cacheGains(VisSet& vi) = 0;

    // write out the gains that would be applied to a given Measurement Set.
    // file is the name of the calibration table to write. 
    // ms is the MeasurementSet which will be read through to discover the 
    // proper times and telescope configurations; it will be resorted, but 
    // otherwise it is not modified.  
    virtual void store(const String& file, MeasurementSet &ms, Bool append) = 0;

    // add the noise for a given antenna and feed to the given gains.
    //   gains is a 2x2 matrix containing actual gains (presumably for the 
    //     antenna and feed specified).  Each row contains a gain for one 
    //     polarization; the columns give the amplitude and phase of the 
    //     gains. 
    //   antenna is the (zero-based) index of the antenna to apply
    //     the noise for.
    //   feed is the (zero-based) index of the receptor to apply
    //     the noise for.
    Bool applyNoise(SquareMatrix<Float, 2> &gains, uInt antenna, uInt feed, 
		    uInt spw) const;

    // Apply the Gradients to the visibilities
    // Since this Jones is not solvable, this default implementation does 
    // nothing.
    virtual VisBuffer& applyGradient(VisBuffer & vb, const Vector<int> &,
				     Int, Int, Int);

    // add data to the running sums used to form the gradient.  
    // Since this Jones is not solvable, this default implementation does 
    // nothing.
    virtual void addGradients(const VisBuffer&, Int, 
			      const Antenna&, const Vector<Float>&, 
			      const Vector<Float>&, const Vector<mjJones2>&, 
			      const Vector<mjJones2F>&);

    // solve for the gains using the calculated gradients
    // Since this Jones is not solvable, this default implementation does 
    // always returns False.
    virtual Bool solve(VisEquation &ve) { return False; }

protected:

    // Assignment.  This is made protected because we don't want to be able
    // to set, say, a BJones to a GJones.
    virtual NewSimVisJones& operator=(const NewSimVisJones& other);

    // resize the noise configuration containers.  Obviously, this should
    // be called whenever the number of antennae or feeds change.
    void resizeNoiseConfig(uInt nants, uInt nfeeds, uInt nspw);

    // Return true if polarization re-sequencing required
    static Bool polznSwitch(const VisBuffer& vb);

    // Re-sequence to (XX,XY,YX,YY) or (RR,RL,LR,LL)
    static void polznMap(CStokesVector& vis);

    // Re-sequence to (XX,YY,XY,YX) or (RR,LL,RL,LR)
    static void polznUnmap(CStokesVector& vis);

private:

    // a secondary seed that is unique for each type of SimVisJones.
    // Subclasses should override this value.  The purpose is to help ensure
    // that the random number generators for different SimVisJones used
    // in the same VisEq are really independent.  (This is probably not 
    // necessary, but it's a "t to be crossed".)
    Int typeseed_p;

    // random number generator
    MLCG rand_p;

    // the noise configuration: the outer Matrix has a shape of (nants, nfeeds)
    // with each element containing a 2x2 inner Matrix.  The first axis of the 
    // inner matrix represents the receptor index; the 2nd, amplitude and phase.
    // The elements of the inner matrix is a pointer to either a Uniform or
    // Normal distribution or to null.
    Cube<Matrix<CountedPtr<Random> > > dist_p;
};

// <summary> 
// A container for antenna gains
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="" tests="" demos="">

// <etymology>
// This class caches antenna gains for use by the SimVis classes.
// </etymology>
//
// <synopsis> 
// This class assumes that 
// </synopsis> 
//
// <motivation>
// AIPS++ containers can be tricky when managing complex types; it helps
// to manage the memory in a separate type
// </motivation>
//
// <todo asof="02/02/14">
// </todo>

class SimAntGainCache {
public:
    SimAntGainCache() : cache_p() { }
    SimAntGainCache(Int nants, Int nfeeds, Int spwins) 
	: cache_p(nants, nfeeds, spwins), def_p(Complex(0.0, 0.0))
    { 
	cache_p = 0;
    }

    ~SimAntGainCache() { clear(); }

    // reshape the cache.  Any previously stored gains will be cleared
    void resize(Int nants, Int nfeeds, Int spwins) { 
	if (! cache_p.shape().isEqual(IPosition(3, nants, nfeeds, spwins))) {
	    if (cache_p.nelements() > 0) clear();
	    cache_p.resize(nants, nfeeds, spwins);
	    cache_p = 0;
	}
    }

    // return True if a gain matrix has been cached for a given time.  
    Bool isDefined(Int ant, Int feed, Int spw, Double time) {
	OrderedMap<Double, mjJones2*> *map = cache_p(ant, feed, spw);
	if (! map) return False;
	return ((*map)(time) != 0);
    }

    // return the cached gain matrix for the given time.  If no matrix is 
    // found for this time, the default matrix is returned.  By updating
    // the values in this matrix implicitly sets the gain in the cache.
    mjJones2& getGain(Int ant, Int feed, Int spw, Double time) {
	mjJones2 *out = 0;
	OrderedMap<Double, mjJones2*> *map = cache_p(ant, feed, spw);
	if (! map) {
	    map = new OrderedMap<Double, mjJones2*>(0, 10);
	    map->incr(10);
	    cache_p(ant, feed, spw) = map;
	}

	out = (*map)(time);
	if (! out) {
	    out = new mjJones2(def_p);
	    map->define(time, out);
	}

	return *out;
    }

    // return the cached gain matrix for the given time.  If no matrix is 
    // found for this time, the default matrix is returned
    mjJones2& operator()(Int ant, Int feed, Int spw, Double time) {
	return getGain(ant, feed, spw, time);
    }

    void invalidate(Int ant, Int feed, Int spw) {
	OrderedMap<Double, mjJones2*> *map = cache_p(ant, feed, spw);
	if (map) clearMap(*map);
    }

    // clear the entire contents of the cache
    void clear();

private:
    void clearMap(OrderedMap<Double, mjJones2*> &map) {
	mjJones2 *j=0;
	MapIter<Double, mjJones2*> miter(map);
	for(; ! miter.atEnd(); ++miter) {
	    if ((j = miter.getVal()) != 0) delete j;
	    j = 0;
	}
    }

    Cube<OrderedMap<Double, mjJones2*>*> cache_p;
    mjJones2 def_p;
};

// <summary> 
// Model multiplicative gain errors for the VisEquation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimVisJones">SimVisJones</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> module
//   <li> <linkto class="VisEquation">VisEquation</linkto> module
// </prerequisite>
//
// <etymology>
// SimGJones describes random multiplicative gains to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. 
// The Jones matrices are diagonal.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="VisJones">VisJones</linkto>
// for how SimVisJones is to be used.
// </synopsis> 
//
// <motivation>
// The properties of a multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class NewSimTimeDepVisJones : public NewSimVisJones {
public:

    // create and initiallize a Jones matrix with no noise added.
    NewSimTimeDepVisJones(Int typeseed=0, Int seed=0);

    // create a Jones matrix for a given number of antennae, feeds, and
    // spectral windows.  The reftime should be in seconds in the frame
    // used by the VisBuffer; this time is used as "time0", the starting time, 
    // when applying functions set via setTimeDep().
    NewSimTimeDepVisJones(uInt nants, uInt nfeeds, uInt nspw, Double starttime,
		       Int typeseed=0, Int seed=0);

    // Create a copy of another SimVisJones.
    NewSimTimeDepVisJones(const NewSimTimeDepVisJones& other);

    // Construct a Jones matrix ready to apply unity gains to a given 
    // VisibilitySet.  That is, the gain 
    // amplitude will be 1, and the phase shift, 0.  Clients should 
    // use setTimeDep(), setNoise(), and setDelayError() to vary the gain.
    //   vs is the VisSet to initialize to.
    //   seed is used to seed the random number generator used to create 
    //     noise (if noise is added later).  
    //   slotintv is an interval of time (in seconds) over which the gain 
    //     is considered constant.  If <= 0, no such interval is assumed, and
    //     gains are integrated over the integration time.  slotintv is a 
    //     somewhat artificial constraint but might be helpful in calibration
    //     testing as it mirrors the practice of binning time into slots.
    NewSimTimeDepVisJones(VisSet& vs, Int typeseed=0, Int seed=0, 
		       Double slotintv=0);

    // delete this SimVisJones
    virtual ~NewSimTimeDepVisJones();

    // set the number of antennae, feeds, and spectral windows this Jones
    // is configured to handle.  This needs to be called before any real 
    // calculations can be done--that is, prior to any call to apply() or
    // applyInv or any of the set...() routines.  This usually allows this
    // object to set its internal storage appropriately.  
    virtual void setShape(Int nants, Int nfeeds, Int nspwins) {
	NewSimVisJones::setShape(nants, nfeeds, nspwins);
	timedep_p.resize(IPosition(5, nants, nfeeds, nspwins, 2, 2));
	antGainCache_p.resize(nants, nfeeds, nspwins);
	intGainCache_p.resize(IPosition(4, nants, nants, nfeeds, nfeeds));
    }

    // reset the reference time.  reftime should be in seconds in the frame
    // used by the VisBuffer; this time is used as "t=0", the starting time,
    // when applying functions set via setTimeDep().    
    virtual void setRefTime(Double reftime) {
	reftime_p = reftime;
    }

    // set the function that will control the systematic variation of the 
    // gains as a function of Time.  This overrides any constant gain set
    // during construction.
    //   antenna is the (zero-based) index of the antenna to update
    //     or a negative value to update all antennae.
    //   receptor is the (zero-based) index of the receptor to update or 
    //     negative to update all receptors simultaneously; since only 2x2
    //     Jones matrices are currently supported, this should be less than 2.
    //   comp is either SimGJones::amp or SimGJones::phase (defined in parent 
    //     class, SimVisJones) to indicate which component of the gain 
    //     should be made to vary.
    //   func is the function that should be used to calculate the gain as 
    //     a function of time.
    //   feed is the (zero-based) index of the feed to update
    //     or negative to update all the feeds simultaneously.
    void setTimeDep(Int antenna, Int receptor, Component comp, 
		    const Function1D<Float>& func, Int feed=-1, 
		    Int spw=-1);

    // remove the systematic time dependence of a component of the gain.
    //   antenna is the (zero-based) index of the antenna to update
    //     or a negative value to update all antennae.
    //   receptor is the (zero-based) index of the receptor to update or 
    //     negative to update all receptors simultaneously; since only 2x2
    //     Jones matrices are currently supported, this should be less than 2.
    //   comp is either SimGJones::amp or SimGJones::phase (defined in parent 
    //     class, SimVisJones) to indicate which component of the varying gain 
    //     should be removed.
    //   feed is the (zero-based) index of the feed to update
    //     or negative to update all the feeds simultaneously.
    void removeTimeDep(Int antenna, Int receptor, Component comp, 
		       Int feed=-1, Int spw=-1);

    // Returns true if a delay error (a frequency-dependent effect) has 
    // been set
    virtual Bool freqDep() { return False; }

    // Apply this Jones to some visibility data.  vb is the buffer containing
    // the data; that same buffer is returned.
    virtual VisBuffer& apply(VisBuffer& vb);

    // Apply the inverse of this Jones to some visibility data.  vb is the 
    // buffer containing the data; that same buffer is returned.
    virtual VisBuffer& applyInv(VisBuffer& vb);

    // write out the gains that would be applied to a given Measurement Set.
    // file is the name of the calibration table to write. 
    // ms is the MeasurementSet which will be read through to discover the 
    // proper times and telescope configurations; it will be resorted, but
    // otherwise, it is not modified.  
    virtual void store(const String& file, MeasurementSet &ms, Bool append);

protected: 
    // Assignment.  This is made protected because we don't want to be able
    // to set, say, a BJones to a GJones.
    virtual NewSimTimeDepVisJones& operator=(const NewSimTimeDepVisJones& other);

    void invalidateCache(uInt ant, uInt feed, uInt spwin) {
	antGainCache_p.invalidate(ant, feed, spwin);
	intGainCache_p = CountedPtr<mjJones4>();
	intInvGainCache_p= CountedPtr<mjJones4>();
    }
    void clearCache();
    void checkIntCacheTime(Double time) {
	if (time-intCacheTime_p > DBL_EPSILON) {
	    intGainCache_p = CountedPtr<mjJones4>();
	    intInvGainCache_p= CountedPtr<mjJones4>();
	    intCacheTime_p = time;
	}
    }

    // return a gain for a given antenna, feed, and spectral window
    const mjJones2& getAntGain(Int ant, Int feed, Int spw, Double time) {
	// adjust time: normalize to the start time; if we're binning 
	// the gains into slots, change time to the middle of a slot 
	// interval.
	time -= reftime_p;
	if (slotintv_p > 0) 
	    time = slotintv_p/2.0 + floor(time/slotintv_p)*slotintv_p;

	if (! antGainCache_p.isDefined(ant, feed, spw, time))
	    cacheAntGain(ant, feed, spw, time);
	return antGainCache_p(ant, feed, spw, time);
    }

    // return a gain for a given interferometer.  This calls cacheAntGain().
    const mjJones4& getIntGain(Int ant1, Int feed1, Int ant2, Int feed2, 
			       Int spw, Double time);

    // return the inverse of the gain for a given interferometer.  This 
    // calls cacheAntGain().
    const mjJones4& getIntInvGain(Int ant1, Int feed1, Int ant2, Int feed2, 
				  Int spw, Double time);

    // calculate and cache Gains for a given antenna at a given time 
    virtual mjJones2& calcAntGain(mjJones2 &jones, uInt ant, uInt feed, 
				  uInt spw, Double time) = 0;

    // return an IPosition to use as an Array index for retreiving the 
    // time dependence function.
    IPosition& settdi(uInt ant, uInt feed, uInt spw, uInt rec, uInt comp) {
	tdi_p(0) = ant;
	tdi_p(1) = feed;
	tdi_p(2) = spw;
	tdi_p(3) = rec;
	tdi_p(4) = comp;
	return tdi_p;
    }

    LogIO log_p;

    // the collection of time-dependent functions
    Array<CountedPtr<Function1D<Float> > > timedep_p;
    IPosition tdi_p;

private:
    static uInt numberFeeds(VisSet &vs) {
	return vs.iter().msColumns().feed().nrow();
    }
    void initFromVisSet(VisSet &vs, Double slotintv);
    IPosition& setigi(uInt ant1, uInt ant2, uInt feed1, uInt feed2) {
	igi_p(0) = ant1;
	igi_p(1) = ant2;
	igi_p(2) = feed1;
	igi_p(3) = feed2;
	return igi_p;
    }
    const mjJones2& cacheAntGain(uInt ant, uInt feed, uInt spw, Double time) {
	mjJones2 &jones = antGainCache_p(ant, feed, spw, time);
	calcAntGain(jones, ant, feed, spw, time);
	return jones;
    }

    // the gain cache
//    Cube<OrderedMap<Double, CountedPtr<mjJones2> > > antGainCache_p;
    SimAntGainCache antGainCache_p;
    Array<CountedPtr<mjJones4> > intGainCache_p;
    Array<CountedPtr<mjJones4> > intInvGainCache_p;
    IPosition igi_p;
    Double intCacheTime_p;

    // time handling
    Double reftime_p, slotintv_p;
};

// <summary> 
// Model multiplicative gain errors for the VisEquation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimVisJones">SimVisJones</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> module
//   <li> <linkto class="VisEquation">VisEquation</linkto> module
// </prerequisite>
//
// <etymology>
// SimGJones describes random multiplicative gains to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. 
// The Jones matrices are diagonal.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="VisJones">VisJones</linkto>
// for how SimVisJones is to be used.
// </synopsis> 
//
// <motivation>
// The properties of a multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class NewSimGJones : public NewSimTimeDepVisJones {
public:

    NewSimGJones(Int seed=0);

    // Construct a Jones matrix that applies unity gain.  That is, gain 
    // amplitude is 1, and the phase shift is 0.  Clients should 
    // use setTimeDep(), setNoise(), and setDelayError() to vary the gain.
    //   nants is the number of antennae to support, 
    //   nfeeds is the number of feeds per antenna, and 
    //   spw is the number of spectral windows.
    //   seed is an optional seed for the noise generator.
    NewSimGJones(uInt nants, uInt nfeeds, uInt spw, Double reftime, Int seed=0);

    // Construct a Jones matrix ready to apply unity gains to a given 
    // VisibilitySet.  That is, the gain 
    // amplitude will be 1, and the phase shift, 0.  Clients should 
    // use setTimeDep(), setNoise(), and setDelayError() to vary the gain.
    //   vs is the VisSet to initialize to.
    //   seed is used to seed the random number generator used to create 
    //     noise (if noise is added later).  
    //   slotintv is an interval of time (in seconds) over which the gain 
    //     is considered constant.  If <= 0, no such interval is assumed, and
    //     gains are integrated over the integration time.  slotintv is a 
    //     somewhat artificial constraint but might be helpful in calibration
    //     testing as it mirrors the practice of binning time into slots.
    NewSimGJones(VisSet& vs, Int seed=0, Double slotintv=0);

    // Construct a Jones matrix ready to apply a constant gain to a given 
    // VisibilitySet.  The gains set with this constructor will apply to all
    // antennae and feeds.
    // Applications should use setTimeDep(), setNoise(), and setDelayError() 
    // to vary the gain.  
    //   vs is the VisSet to initialize to.  
    //   The gain amplitude for the first receptor is given by amp0, and its 
    //     corresponding  phase shift is given by phase0.  
    //   amp1 and phase1 similarly set the gain for the second receptor.  
    //   seed is used to seed the random number generator used to create 
    //     noise (if noise is added later).  
    //   slotintv is an interval of time (in seconds) over which the gain 
    //     is considered constant.  If <= 0, no such interval is assumed, and
    //     gains are integrated over the integration time.  slotintv is a 
    //     somewhat artificial constraint but might be helpful in calibration
    //     testing as it mirrors the practice of binning time into slots.
    NewSimGJones(VisSet& vs, Double amp0, Double phase0, 
              Double amp1, Double phase1, Int seed=0, Double slotintv=0);

    NewSimGJones(const NewSimGJones &other);

    virtual NewSimGJones& operator=(const NewSimGJones &other);

    // delete this Jones matrix
    virtual ~NewSimGJones();

    // return G as the type of this Jones matrix.
    virtual Type type() { return G;};

protected:
    // calculate and cache Gains for a given antenna at a given time 
    virtual mjJones2& calcAntGain(mjJones2 &jones, uInt ant, uInt feed, 
				  uInt spw, Double time);

private:
    void setdefgains(Double amp0, Double phase0, Double amp1, Double phase1) {
	defgains_p(0,0) = amp0;
	defgains_p(1,0) = amp1;
	defgains_p(0,1) = phase0;
	defgains_p(1,1) = phase1;
    }

    Matrix<Float> defgains_p;
};

// <summary> 
// Model multiplicative gain errors for the VisEquation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimVisJones">SimVisJones</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> module
//   <li> <linkto class="VisEquation">VisEquation</linkto> module
// </prerequisite>
//
// <etymology>
// SimGJones describes random multiplicative gains to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. 
// The Jones matrices are diagonal.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="VisJones">VisJones</linkto>
// for how SimVisJones is to be used.
// </synopsis> 
//
// <motivation>
// The properties of a multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class NewSimDJones : public NewSimTimeDepVisJones {
public:

    NewSimDJones(Int seed=0);

    // Construct a Jones matrix that applies unity gain.  That is, gain 
    // amplitude is 1, and the phase shift is 0.  Clients should 
    // use setTimeDep(), setNoise(), and setDelayError() to vary the gain.
    //   nants is the number of antennae to support, 
    //   nfeeds is the number of feeds per antenna, and 
    //   spw is the number of spectral windows.
    //   seed is an optional seed for the noise generator.
    NewSimDJones(uInt nants, uInt nfeeds, uInt spw, Double reftime, Int seed=0);

    // Construct a Jones matrix ready to apply unity gains to a given 
    // VisibilitySet.  That is, the gain 
    // amplitude will be 1, and the phase shift, 0.  Clients should 
    // use setTimeDep(), setNoise(), and setDelayError() to vary the gain.
    //   vs is the VisSet to initialize to.
    //   seed is used to seed the random number generator used to create 
    //     noise (if noise is added later).  
    //   slotintv is an interval of time (in seconds) over which the gain 
    //     is considered constant.  If <= 0, no such interval is assumed, and
    //     gains are integrated over the integration time.  slotintv is a 
    //     somewhat artificial constraint but might be helpful in calibration
    //     testing as it mirrors the practice of binning time into slots.
    NewSimDJones(VisSet& vs, Int seed=0, Double slotintv=0);

    // Construct a Jones matrix ready to apply a constant gain to a given 
    // VisibilitySet.  The gains set with this constructor will apply to all
    // antennae and feeds.
    // Applications should use setTimeDep(), setNoise(), and setDelayError() 
    // to vary the gain.  
    //   vs is the VisSet to initialize to.  
    //   The gain amplitude for the first receptor is given by amp0, and its 
    //     corresponding  phase shift is given by phase0.  
    //   amp1 and phase1 similarly set the gain for the second receptor.  
    //   seed is used to seed the random number generator used to create 
    //     noise (if noise is added later).  
    //   slotintv is an interval of time (in seconds) over which the gain 
    //     is considered constant.  If <= 0, no such interval is assumed, and
    //     gains are integrated over the integration time.  slotintv is a 
    //     somewhat artificial constraint but might be helpful in calibration
    //     testing as it mirrors the practice of binning time into slots.
    NewSimDJones(VisSet& vs, Double amp0, Double phase0, 
              Double amp1, Double phase1, Int seed=0, Double slotintv=0);

    NewSimDJones(const NewSimDJones &other);

    virtual NewSimDJones& operator=(const NewSimDJones &other);

    // delete this Jones matrix
    virtual ~NewSimDJones();

    // return G as the type of this Jones matrix.
    virtual Type type() { return D;};

protected:
    // calculate and cache Gains for a given antenna at a given time 
    virtual mjJones2& calcAntGain(mjJones2 &jones, uInt ant, uInt feed, 
				  uInt spw, Double time);

private:
    void setdefgains(Double amp0, Double phase0, Double amp1, Double phase1) {
	defgains_p(0,0) = amp0;
	defgains_p(1,0) = amp1;
	defgains_p(0,1) = phase0;
	defgains_p(1,1) = phase1;
    }

    Matrix<Float> defgains_p;
};

inline VisBuffer& NewSimVisJones::applyGradient(VisBuffer & vb,
					     const Vector<int> &,
					     Int, Int, Int) 
{return vb;}

inline void NewSimVisJones::addGradients(const VisBuffer&, Int, const Antenna&,
				      const Vector<Float>&, 
				      const Vector<Float>&,
				      const Vector<mjJones2>&, 
				      const Vector<mjJones2F>&) {}



} //# NAMESPACE CASA - END

#endif

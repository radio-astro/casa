//# SimFeed.h: container for containing feed descriptions in memory
//# Copyright (C) 2002,2003
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

#ifndef SIMULATORS_SIMFEED_H
#define SIMULATORS_SIMFEED_H

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/Stokes.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MSFeed;
class MSFeedColumns;

// <summary> a container for data destined for an MS FEED record </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for data describing a signal feed for a simulated
// telescope.
// </etymology>
//
// <synopsis> 
// This class describes a feed in terms of the data that make up a record
// in the FEED table.  In the MS data 
// model, a feed can carry the signals for one or more polarizations.  This
// class records those types as well as more complex characteristics of the
// feed.  
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimTelescope">SimTelescope</linkto> class.  <p>
// 
// The most important part of the feed description is the set of 
// polarization types.  These are set at construction time or with 
// setPolTypes().  The "key" is a set of data that uniquely identifies this
// feed within a set of records in a FEED table and is made up of a time 
// stamp, a time interval, and a set of IDs; these are all set simultaneously
// with setKey().  Currently with this class, a feed is restricted to be 
// associated with either a single antenna (and/or window) or all antennas
// currently defined.  The more complex information, such as the feed postion 
// and beam offset, can also be set after construction, via various set 
// functions. <p>
//
// A row marker, set with setRow(), is used by 
// <linkto class="SimFeedList">SimFeedList</linkto> to indicate that this 
// SimFeed has been written out to a FEED table.  In general, a single 
// SimFeed will be written out as a series of rows in the FEED table, usually 
// one for each antenna.  Thus, the row marker represents the first row 
// associated with this SimFeed. <p>
//
// Although this class has several non-const functions that update the 
// internal data, it is not intended that the data be changed arbitrarily 
// at any time.  The complexity of the data (and the fact that much of it
// is optional) makes setting it via update functions more convenient than
// via a constructor.  Care should be taken to avoid updating the object
// after it has been copied to disk (by the 
// <linkto class="SimFeedList">SimFeedList</linkto> class).  A negative
// value returned from getRow() indicates that the item has not yet been
// written out. <p>
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a FEED table.  
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS FEED 
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
//   <li> Because this class was written first, it behaves a little different
//        from the other Sim* classes; it should probably be homogenized a 
//        bit.
// </todo>
class SimFeed {
public:

    // create the record container.  
    //   nrec is the number of receptors associated with this feed.
    //   types is a String containing the polarization codes for each 
    //      receptor in their proper order (with no intervening spaces); 
    //      the length of the string must either be 0 (to be set later 
    //      with setPolTypes()) or equal to number of receptors.
    //   ant is the ID for the antenna associated with this feed; a value of -1 
    //      means this applies to all antennae.
    //   spw is the ID for the spectral window associated with this feed; a 
    //      value of -1 means that this applies to all windows.
    SimFeed(uInt nrec=2, String types="", Int ant=-1, Int spw=-1) : 
	nrecp_p(nrec), antId_p(ant), spwId_p(spw), beamId_p(-1), row_p(-1), 
	pols_p(), position_p(0), beamOff_p(0), polResp_p(0), angle_p(0) 
    { 
	if (types.length() > 0) setPolTypes(types);
    }

    // make a copy of the record
    SimFeed(const SimFeed& f) : nrecp_p(f.nrecp_p), 
	antId_p(f.antId_p), spwId_p(f.spwId_p), beamId_p(f.beamId_p), row_p(-1),
	pols_p(f.pols_p), position_p(f.position_p), beamOff_p(f.beamOff_p), 
	polResp_p(f.polResp_p), angle_p(f.angle_p) 
    { }

    SimFeed& operator=(const SimFeed& f);

    virtual ~SimFeed() { deletePointers(); }

    // set the values that make up the lookup key for this feed.  Negative 
    // values for ant and spw mean the record applies to all antennae and 
    // windows, respectively.  A value of 0 for time defaults to the start
    // time.
    void setKey(uInt feed, Int ant=-1, Int spw=-1, 
		Double time=0, Double interval=0) 
    {
	feedId_p = feed;
	antId_p = ant;
	spwId_p = spw;
	time_p = time;
	intv_p = interval;
//	beamId_p = beam;
    }

    uInt numReceptors() const { return nrecp_p; }
    uInt feedId() const { return feedId_p; }
    Int antId() const { return antId_p; }
    Int spwId() const { return spwId_p; }
    Int beamId() const { return beamId_p; }
    Double time() const { return time_p; }
    Double interval() const { return intv_p; }

    uInt nrows(uInt nants) const { return ((antId_p < 0) ? nants : 1); }

    void setPolTypes(const String &types) {
	if (types.length() < nrecp_p) 
	    throw AipsError(String("Insufficient no. of pols passed in '") 
			    + types + "'; expected " + nrecp_p + ", got " + 
			    types.length());
	pols_p = types;
    }

    const String& getPolTypes() const { return pols_p; }

    // set the Beam ID
    void setBeamID(Int id) { beamId_p = id; }

    Matrix<Double>& beamOffset() {
	if (! beamOff_p) beamOff_p = new Matrix<Double>(2, nrecp_p, 0.0);
	return *beamOff_p;
    }

    Matrix<Complex>& polResponse() {
	if (! polResp_p) {
	    polResp_p = new Matrix<Complex>(nrecp_p, nrecp_p);
	    polResp_p->diagonal() = Complex(1.0, 0.0);
	}
	return *polResp_p;
    }

    void setPosition(const MPosition &pos) { position_p = new MPosition(pos); }

    Vector<Double>& receptorAngle() {
	if (! angle_p) angle_p = new Vector<Double>(nrecp_p, 0.0);
	return *angle_p;
    }
    
    // get and set the row ID, the row of the first record with this feedID.  
    // An ID less than zero means that this record has not yet been recorded.
    // <group>
    Int getRow() const { return row_p; }
    void setRow(Int id) { row_p = id; }
    // </group>

    uInt write(MSFeed& msf, MSFeedColumns& msfc, uInt nant=0);

protected:
    
private:
    void deletePointers() {
	if (beamOff_p) { delete beamOff_p; beamOff_p = 0; }
	if (polResp_p) { delete polResp_p; polResp_p = 0; }
	if (position_p) { delete position_p; position_p = 0; }
	if (angle_p) { delete angle_p; angle_p = 0; }
    }

    uInt feedId_p, nrecp_p;
    Int antId_p, spwId_p, beamId_p, row_p;
    Double time_p, intv_p;

    String pols_p;

    MPosition *position_p;
    Matrix<Double> *beamOff_p;
    Matrix<Complex> *polResp_p;
    Vector<Double> *angle_p;
};

// <summary> a container for data destined for an MS FEED table </summary>

// <use visibility=local>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimFeed">SimFeed</linkto>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// <etymology>
// This class holds a list of <linkto class="SimFeed">SimFeed</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimFeed">SimFeed</linkto> 
// instances which, as a group, describes a spectrometer set-up (typical
// of correlation spectrometers).  
// When using this class, one does not create SimFeed objects directly,
// but rather implicitly via the addFeed() function.
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimTelescope">SimTelescope</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// feed descriptions to a Measurement Set.  This is done with the flush()
// function.  When it writes the data from a SimFeed in its list to the MS,
// it sets the row number of the first feed record via setRow();
// this is used as a flag indicating that the SimFeed as been flushed.  This
// allows one to later add additional feeds to the list; when flush() is 
// called again, only the new SimFeed data are written out.  This, of course, 
// assumes that the same output MSFeed is passed to the flush() 
// function each time.  If you want to write all the data to a new 
// FEED table, you can call clearIds() to clear all the row 
// markers.  <p>
// 
// A set of feeds can be read in from a FEED table as well using 
// the initFrom() method or the SimFeedList(const MSFeed&, ...) 
// constructor.  These will implicitly set the row markers from the input 
// table.  This allows one to add new feeds to the already recorded set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a FEED table.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS FEED
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimFeedList {
public:
    // create an empty list
    SimFeedList(uInt initsz=2, uInt stepsz=4) 
	: n_p(0), chnk_p(stepsz), recs_p(initsz, 0) { }

    // create a copy of another list
    // <group>
    SimFeedList(const SimFeedList& f, uInt stepsz=4) 
	: n_p(0), chnk_p(stepsz), recs_p(f.recs_p.nelements(), 0) 
    {
	for(uInt i=0; i < recs_p.nelements(); i++)
	    recs_p[i] = new SimFeed(*(f.recs_p[i]));
    }
    SimFeedList& operator=(const SimFeedList& f);
    // </group>

    // load the feeds described in the given FEED table
    SimFeedList(const MSFeed& fdt, uInt stepsz=4);

    virtual ~SimFeedList() { deleteRecs(); }

    // count the number of rows assuming a given number of antennae
    uInt countRows(uInt nants, Bool unflushed=False) const {
	uInt nrows = 0;
	for(uInt i=0; i < recs_p.nelements(); i++) {
	    if (unflushed || recs_p[i]->getRow() >= 0) 
		nrows += recs_p[i]->nrows(nants);
	}
	return nrows;
    }

    // return the number of feeds described in this list
    uInt numFeeds() const { return n_p; }

    // return True if all feeds have the same number of receptors on them
    Bool uniform() const {
	if (recs_p.nelements() == 0) return True;
	Bool nr = recs_p[0]->numReceptors();
	for(uInt i=1; i < recs_p.nelements(); i++) {
	    if (nr != recs_p[i]->numReceptors()) return False;
	}
	return True;
    }

    // add a feed to this list.  nrecp is the number of receptors attached to 
    // the feed.  types lists polarizations for the receptors.  The length
    // of the string is equal to nrecp, and the values are taken from "RLXY".
    SimFeed& addFeed(uInt nrecp, String types) {
	SimFeed *out = new SimFeed(nrecp);
	out->setPolTypes(types);
	add(out);
	return *out;
    }

    // access the i-th SimFeed object in this list.  
    // <group>
    SimFeed& operator[](Int i) { return get(i); }
    const SimFeed& operator[](Int i) const { return get(i); }
    // </group>

    // reset all the row markers used to flag the SimFeed members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimFeeds to the FEED table.  
    // This should be used when writing to a new FEED table, 
    // different from one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    recs_p[i]->setRow(-1);
    }

    // add feed descriptions from a Measurement set 
    void initFrom(const MSFeed& fdt);

    // write out the FEED table assuming a given number of antennae
    // <group>
    uInt write(MSFeed &msf, uInt nants) const;
    uInt flush(MSFeed& msf, uInt nants=0) const;
    // </group>

private:
    void deleteRecs() {
	for(uInt i=0; i < recs_p.nelements(); i++) {
	    if (recs_p[i]) delete recs_p[i];
	}
    }

    SimFeed& get(Int i) const {
        if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(recs_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;

    void add(SimFeed *feed) {
	if (n_p == recs_p.nelements()) {
	    recs_p.resize(n_p+chnk_p);
	    for(uInt i=n_p; i < recs_p.nelements(); i++) recs_p[i] = 0;
	}
	recs_p[n_p++] = feed;
    }

    uInt n_p, chnk_p;
    Block<SimFeed*> recs_p;
};


} //# NAMESPACE CASA - END

#endif

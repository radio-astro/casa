//# SimSource.h: a source for simulated observations
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

#ifndef SIMULATORS_SIMSOURCE_H
#define SIMULATORS_SIMSOURCE_H

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MeasurementSet;
class MSSource;

// <summary> a container for data destined for an MS SOURCE table record
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimObservations">SimObservations</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for describing sources to be observed in simulated 
// observations.
// </etymology>
//
// <synopsis> 
// This container describes the sources that are to be observed in a set 
// of simulated observations.  It provides a simpler (subset) representation
// of a source than what is allowed in the MS SOURCE table.  
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// For far-field, non-moving objects, one sets the description at construction
// time with an object name, direction, and, optionally, a code and associated
// spectral window ID.  This data cannot be changed later.  One can later set
// a time stamp and interval if the source is moving, and/or a distance if it
// is in the near-field.  It is intended that this latter extended data would 
// be set once and not changed (particularly after the data have been flushed 
// to disk).
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a SOURCE table.  
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS SOURCE table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimSource {
public:

    // Create a record
    // name    is the name of the source.
    // dir     is the direction to the source
    // code    is an optional classification of the source in terms 
    //            of the role it plays in data processing (see the MS2
    //            spec's defintion of the SOURCE table for details).
    // spWinId is an associated spectral window ID.  If set to -1, the 
    //            source is associated with all windows.  
    SimSource(const String& name, const MDirection& dir, 
	      const String& code="", Int spWinId=-1);

    // create a copy
    // <group>
    SimSource(const SimSource& that);
    SimSource& operator=(const SimSource& that);
    // </group>

    // get and set the row ID, the row in the SOURCE table containing this 
    // record.  An ID less than zero means that this record has not yet 
    // been recorded.
    // <group>
    Int getRow() const { return row_p; }
    void setRow(Int id) { row_p = id; }
    // </group>

    // get and set the SOURCE_ID.  
    // An ID less than zero means that it will be set to the row ID when it 
    // is recorded in the SOURCE table
    // <group>
    Int getId() const { return id_p; }
    void setId(Int id) { id_p = id; }
    // </group>

    // return the source name
    const String &name() const { return name_p; }

    // return the direction to the source
    const MDirection& direction() const { return dir_p; }

    // return the role code for this source
    const String& code() const { return code_p; }

    // set the mid-point of the time interval for which this source
    // description is valid.  
    void setTime(MEpoch t) { time_p = t; }

    // return the mid-point of the time interval for which this source
    // description is valid.  If interval() is < 0, then this time 
    // represents the starting time.  The default time is 0s.  timesec() 
    // returns the time in seconds and, thus, is more convenient for 
    // determining if the time has been set from its default.
    // <group>
    const MEpoch& time() const { return time_p; }
    const Double timeSec() const { return time_p.get("s").getValue("s"); }
    // </group>

    // return the time interval for which this source description is valid.
    // If the interval is < 0, then this description is valid until the 
    // starting time of the next record.
    Double interval() const { return intv_p; }

    // set the time interval for which this source description is valid.
    // If the interval is < 0, then this description is valid until the 
    // starting time of the next record.
    // <group>
    void setInterval(Quantity intv) { intv_p = intv.getValue("s"); }
    void setIntervalSec(Double intv) { intv_p = intv; }
    // </group>

    // return the spectral window associated with this source.  A value
    // less than 0 refers to all windows.
    Int spectralWindow() const { return spwin_p; }

    // get and the distance to the source.  If <= 0.0, the distance is taken
    // to be in the far-field (default).  The units on the value returned by 
    // distancekm() is kilometers.
    // <group>
    Quantity distance() const { return Quantity(dist_p, "km"); }
    Double distancekm() const { return dist_p; }
    void setDistance(const Quantity& dist) { dist_p = dist.getValue("km"); }
    void setDistancekm(Double dist) { dist_p = dist; }
    // </group>


private:
    Int spwin_p, row_p, id_p;
    String name_p, code_p;
    MDirection dir_p;
    MEpoch time_p;
    Double intv_p;
    Double dist_p;  // in km; <= 0 means infinity
};

// <summary> a container for data destined for an MS SOURCE table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimSource">SimSource</linkto>
//   <li> <linkto class="SimObservations">SimObservations</linkto>
// </prerequisite>
//
// <etymology>
// This class holds a list of <linkto class="SimSource">SimSource</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimSource">SimSource</linkto> 
// instances which, as a group, describes a list of sources to observe.
// When using this class, one does not create SimSource objects directly,
// but rather implicitly via the addSource() function.
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// data descriptiosn to a Measurement Set.  This is done with the flush()
// function.  When it writes the data from a SimSource in its list to the MS,
// it sets the row number of the source record via setRow();
// this is used as a flag indicating that the SimSource as been flushed.  This
// allows one to later add additional setups to the list; when flush() is 
// called again, only the new SimSource data are written out.  This, of 
// course, assumes that the same output MSSource is passed to the 
// flush() function each time.  If you want to write all the data to a new 
// SOURCE table, you can call clearIds() to clear all the row 
// markers.  <p>
// 
// A list of polarization setups can be read in from a SOURCE table 
// as well using the initFrom() method or the 
// SimSourceList(const MSSource&, ...) 
// constructor.  These will implicitly set the row markers from the input 
// table.  This allows one to add new setups to the already recorded set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// sources that will ultimately be written to a SOURCE table.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS SOURCE 
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimSourceList {
public:
    // create an empty list
    SimSourceList(uInt initsz=2, uInt stepsz=4)
	: n_p(0), chnk_p(stepsz), rec_p(initsz, 0) {}

    // create a list and fill it with the contents of a SOURCE table
    SimSourceList(const MSSource& srct, uInt stepz=4);

    // create a copy
    // <group>
    SimSourceList(const SimSourceList& t);
    SimSourceList& operator=(const SimSourceList& t);
    // </group>

    ~SimSourceList() { deleteRecs(); }
    
    // reset all the row markers used to flag the SimSource members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimPols to the SOURCE table.  
    // This should be used when writing to a new SOURCE table, 
    // different from one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    rec_p[i]->setRow(-1);
    }

    // access the i-th source description in this list.  
    // <group>
    SimSource& operator[](Int i) { return get(i); }
    const SimSource& operator[](Int i) const { return get(i); }
    // </group>

    // return the number of sources in this list
    Int numSources() const { return n_p; }

    // Create a source description.
    // name    is the name of the source.
    // dir     is the direction to the source
    // code    is an optional classification of the source in terms 
    //            of the role it plays in data processing (see the MS2
    //            spec's defintion of the SOURCE table for details).
    // spWinId is an associated spectral window ID.  If set to -1, the 
    //            source is associated with all windows.  
    SimSource& addSource(const String& name, const MDirection& dir, 
			 const String& code="", Int spWinId=-1)
    {
	SimSource *out = new SimSource(name, dir, code, spWinId);
	add(out);
	return *out;
    }

    // add source descriptions from an MS SOURCE table
    void initFrom(const MSSource& srct);

    // write out all sources that have yet to be written.
    void flush(MSSource& srct);

private:
    void deleteRecs() {
	for(uInt i=0; i < n_p; i++) {
	    if (rec_p[i] != 0) delete rec_p[i];
	    rec_p[i] = 0;
	}
    }
    SimSource& get(Int i) const {
	if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(rec_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;

    void add(SimSource *src) {
	if (n_p == rec_p.nelements()) {
	    rec_p.resize(n_p+chnk_p);
	    for(uInt i=n_p; i < rec_p.nelements(); i++) rec_p[i] = 0;
	}
	rec_p[n_p++] = src;
    }

    uInt n_p, chnk_p;
    Block<SimSource*> rec_p;
};


} //# NAMESPACE CASA - END

#endif

//# SimDoppler.h: a source for simulated observations
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

#ifndef SIMULATORS_SIMDOPPLER_H
#define SIMULATORS_SIMDOPPLER_H

#include <measures/Measures/MFrequency.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MSSpectralWindow;
class MSDoppler;
class MSSource;

// <summary> a container for spectral line information for simulated
// observations.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimObservations">SimObservations</linkto>
//   <li> <linkto class="SimSource">SimSource</linkto>
//   <li> <linkto class="SimSpWin">SimSpWin</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for describing doppler tracking in simulated 
// observations.  
// </etymology>
//
// <synopsis> 
// This container holds data describing a spectral line to be observed,
// along with which spectral windows it can appear in.  This information
// is ultimately destined to be recorded in a Measurement Set:  the spectral
// line--that is, the species name, transition, and rest frequency--will go
// in the SOURCE table; however, the association with spectral windows is 
// recorded in the DOPPLER table.  
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// The spectral line information is set at construction time and cannot be
// updated later.  The only writable data is a row marker (set with setRow())
// which is used by <linkto class="SimDopplerList">SimDopplerList</linkto>
// to flag whether the line has been recorded to an MS, yet.  The marker value 
// represents the first row in the DOPPLER table corresponding to the 
// SimDoppler's spectral line.  Since a line can be associated with multiple
// spectral windows, a single SimDoppler can result in several DOPPLER records
// (as well as SOURCE records).  <p>
//
// Note that when a SimDoppler is created, no check is done to ensure that
// the source ID and spectral window IDs exist or are otherwise valid; in 
// general, this should be done at a higher level.  On the other hand, the 
// <linkto class="SimDopplerList">SimDopplerList</linkto> does check validity 
// when it writes the line data to an MS.  It skips over any SimDopplers 
// with invalid IDs, and it refuses associate a line with a window that has
// already been linked to a different line.  Thus if a spectral line window
// is referenced multiple times, either within a single SimDoppler or across
// several, the first one encountered takes precendence.  
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a DOPPLER table.  It is mainly
// needed as a way to associate spectral window with a particular a spectral 
// line.  The interface is oriented toward what is typical with current 
// instruments and thus is simpler than the interface provided by the MS 
// DOPPLER table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
//   <li> Strictly speaking, the DOPPLER table is meant to exist only 
//        when doppler tracking is used.  However, when this class was 
//        first created, it provided the only mechanism for associating
//        spectral line (recorded in the SOURCE table) with a spectral 
//        window.  This may no longer be necessary; thus, it might be 
//        useful to transform this class into a SimSpLine class.
// </todo>
class SimDoppler {
public:

    // Create a record associating a spectral line with a single 
    // window.
    //   srcId         is the ID of the Source associated with this line
    //   rfreq         is the line's rest frequency.
    //   transition    is a String representation of the species and 
    //                    transition (e.g. "CO J=1-0").
    //   spwid         is the ID of the spectral window to associate this line 
    //                    with.  If this ID is < 0, then it will be associated
    //                    with all the windows defined at the time the line
    //                    data is written out.
    SimDoppler(uInt srcId, const MFrequency& rfreq, 
	       const String& transition, Int spwid=-1) 
	: row_p(-1), src_p(srcId), rfreq_p(rfreq), trans_p(transition),
	  spwids_p(1)
    {
	spwids_p(0) = spwid;
    }

    // Create a record associating a spectral line with a list of
    // windows
    //   srcId         is the ID of the Source associated with this line
    //   rfreq         is the line's rest frequency.
    //   transition    is a String representation of the species and 
    //                    transition (e.g. "CO J=1-0").
    //   spwids        is the list of spectral window IDs to associate this 
    //                    line with.  If any of the IDs is < 0, then it will 
    //                    be associated with all the windows defined at the 
    //                    time the line data is written out.
    SimDoppler(uInt srcId, const MFrequency& rfreq, 
	       const String& transition, const Vector<Int>& spwids) 
	: row_p(-1), src_p(srcId), rfreq_p(rfreq), trans_p(transition),
	  spwids_p(spwids)
    { }

    // create a copy
    // <group>
    SimDoppler(const SimDoppler& s);
    SimDoppler& operator=(const SimDoppler& s);
    // </group>

    ~SimDoppler() { }

    // get and set the row ID, the row in the DOPPLER table containing this 
    // record.  An ID less than zero means that this record has not yet 
    // been recorded.
    // <group>
    Int getRow() const { return row_p; }
    void setRow(Int id) { row_p = id; }
    // </group>

    // return the ID of the source observed in this field
    uInt sourceId() const { return src_p; }

    // return the name of the spectral line transition (e.g. "CO J=1-0");
    const String& transition() const { return trans_p; } 

    // return the rest frequency of the spectral line transition
    const MFrequency& restFrequency() const { return rfreq_p; }

    // return the list of window IDs this line is associated with.
    // An id < 0 indicates that the line is associated with all windows
    const Vector<Int>& spectralWindowIds() { return spwids_p; }

private:
    Int row_p;
    uInt src_p;
    MFrequency rfreq_p;
    String trans_p;
    Vector<Int> spwids_p;
};


// <summary> a container for data destined for an MS DOPPLER table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimDoppler">SimDoppler</linkto>
//   <li> <linkto class="SimSource">SimSource</linkto>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// // <etymology>
// This class holds a list of <linkto class="SimDoppler">SimDoppler</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimDoppler">SimDoppler</linkto> 
// instances which, as a group, describes a list of spectral lines to observe.
// When using this class, one does not create SimDoppler objects directly,
// but rather implicitly via the addDoppler() function.
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// spectral lines to a Measurement Set.  This is done with the flush()
// function which writes to the SOURCE, SPECTRAL_WINDOW, and DOPPLER tables.  
// When a SimDoppler is written out, it sets the row number of the first 
// DOPPLER record corresponding to it via the setRow() function;
// this is used as a flag indicating that the SimDoppler has been flushed.  This
// allows one to later add additional setups to the list; when flush() is 
// called again, only the new SimDoppler data are written out.  This, of 
// course, assumes that the same set of output MS tables are passed to the 
// flush() function each time.  If you want to write all the data to a new 
// DOPPLER table, you can call clearIds() to clear all the row 
// markers.  <p>
// 
// A list of polarization setups can be read in from a DOPPLER table 
// as well using the initFrom() method or the 
// SimDopplerList(const MSDoppler&, const MSSource&, const MSpectralWindow&) 
// constructor.  These will implicitly set the row markers from the input 
// table.  This allows one to add new setups to the already recorded set.
//
// Note that when a SimDoppler is added to the list, no check is done to 
// ensure that the source ID and spectral window IDs exist or are otherwise 
// valid (in general, this should be done at a higher level).  Validity is
// checked, however, when the list is written out to an MS.  It skips over 
// any SimDopplers with invalid IDs, and it refuses associate a line with 
// a window that has already been linked to a different line.  Thus if a 
// spectral line window is referenced multiple times within the list, the 
// first one encountered takes precendence.  
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// spectral lines that will ultimately be written to an MS's SOURCE, DOPPLER 
// and SPECTRAL_WINDOW tables.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS table interfaces.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimDopplerList {
public: 
    // create an empty list
    SimDopplerList(uInt initsz=2, uInt stepsz=4)
	: n_p(0), chnk_p(stepsz), rec_p(initsz, 0) {}

    // create a list and fill it with the contents of a DOPPLER table
    SimDopplerList(const MSDoppler& dopt, const MSSource& srct, 
		   const MSSpectralWindow& spwt, uInt stepz=4);

    // create a copy
    // <group>
    SimDopplerList(const SimDopplerList& t);
    SimDopplerList& operator=(const SimDopplerList& t);
    // </group>

    ~SimDopplerList() { deleteRecs(); }
    
    // reset all the row markers used to flag the SimDoppler members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimFields to the proper tables.  
    // This should be used when writing to a new MS, 
    // different from one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    rec_p[i]->setRow(-1);
    }

    // access the i-th spectral line description in the list.
    // <group>
    SimDoppler& operator[](Int i) { return get(i); }
    const SimDoppler& operator[](Int i) const { return get(i); }
    // </group>

    // return the number of descriptions in this list.
    Int numDopplers() const { return n_p; }

    // Create a record associating a spectral line with a one or more
    // windows.
    //   srcId         is the ID of the Source associated with this line
    //   rfreq         is the line's rest frequency.
    //   transition    is a String representation of the species and 
    //                    transition (e.g. "CO J=1-0").
    //   spwid         is the ID of the spectral window to associate this line 
    //                    with.  If this ID is < 0, then it will be associated
    //                    with all the windows defined at the time the line
    //                    data is written out.
    //   spwids        is the list of spectral window IDs to associate this 
    //                    line with.  If any of the IDs is < 0, then it will 
    //                    be associated with all the windows defined at the 
    //                    time the line data is written out.
    // <group>
    SimDoppler& addDoppler(uInt srcId, const MFrequency& rfreq, 
			   const String& transition, Int spwid=-1)
    {
	SimDoppler *out = new SimDoppler(srcId, rfreq, transition, spwid);
	add(out);
	return *out;
    }
    SimDoppler& addDoppler(uInt srcId, const MFrequency& rfreq, 
			   const String& transition, 
			   const Vector<Int>& spwids)
    {
	SimDoppler *out = new SimDoppler(srcId, rfreq, transition, spwids);
	add(out);
	return *out;
    }
    SimDoppler& addDoppler(uInt srcId, const MFrequency& rfreq, 
			   const String& transition, 
			   const Vector<uInt>& spwids)
    {
	Vector<Int> tmp(spwids.nelements());
	for(uInt i=0; i < tmp.nelements(); i++) tmp(i) = spwids(i);
	SimDoppler *out = new SimDoppler(srcId, rfreq, transition, tmp);
	add(out);
	return *out;
    }
    // </group>

    // add spectral line descriptions from an MS DOPPLER table
    void initFrom(const MSDoppler& dopt, const MSSource& srct, 
		  const MSSpectralWindow& spwt);

    // write out all spectral lines that have yet to be written
    void flush(MSDoppler& dopt, MSSource& srct, MSSpectralWindow& spwt);

private:
    void deleteRecs() {
	for(uInt i=0; i < n_p; i++) {
	    if (rec_p[i] != 0) delete rec_p[i];
	    rec_p[i] = 0;
	}
    }
    SimDoppler& get(Int i) const {
	if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(rec_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;

    void add(SimDoppler *dop) {
	if (n_p == rec_p.nelements()) {
	    rec_p.resize(n_p+chnk_p);
	    for(uInt i=n_p; i < rec_p.nelements(); i++) rec_p[i] = 0;
	}
	rec_p[n_p++] = dop;
    }

    uInt n_p, chnk_p;
    Block<SimDoppler*> rec_p;
};


} //# NAMESPACE CASA - END

#endif
